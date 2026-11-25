#include "NFEnemyBase.h"
#include "NewFPS.h"
#include "NFEnemyDirector.h"
#include "NFHealthComponent.h"
#include "NFWetnessComponent.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Navigation/PathFollowingComponent.h"
#include "Net/UnrealNetwork.h"

namespace NFEnemy
{
	/** Colour parameter of /Engine/BasicShapes/BasicShapeMaterial. */
	const FName ColorParameter(TEXT("Color"));
	/** Seconds between target re-evaluations. */
	const float TargetSearchInterval = 0.5f;
}

ANFEnemyBase::ANFEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UNFHealthComponent>(TEXT("HealthComponent"));
	WetnessComponent = CreateDefaultSubobject<UNFWetnessComponent>(TEXT("WetnessComponent"));

	// A plain AI controller is all the state machines need (NavMesh MoveTo + focus).
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// The capsule is for movement only; bullets go through it to the hit volumes of the subclass.
	GetCapsuleComponent()->SetCollisionResponseToChannel(NF_ECC_Weapon, ECR_Ignore);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Face what the AI controller focuses on (the target), turning at a readable speed.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bUseControllerDesiredRotation = true;
	Movement->bOrientRotationToMovement = false;
	Movement->RotationRate = FRotator(0.f, 540.f, 0.f);
}

void ANFEnemyBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANFEnemyBase, VisualState);
	DOREPLIFETIME(ANFEnemyBase, VisualAttackId);
}

void ANFEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnDeath.AddUniqueDynamic(this, &ANFEnemyBase::HandleDeath);

	if (HasAuthority())
	{
		HealthComponent->OnDamageTaken.AddUObject(this, &ANFEnemyBase::HandleDamageTakenNative);
		if (UNFEnemyDirector* Director = GetDirector())
		{
			Director->RegisterEnemy(this);
		}
	}
	else
	{
		// Clients do not think.
		SetActorTickEnabled(false);
	}

	// A client that joins late can get an enemy that is already dead.
	if (HealthComponent->IsDead())
	{
		HandleDeath(HealthComponent, nullptr);
	}
	else
	{
		NotifyVisualStateChanged();
	}
}

void ANFEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		if (UNFEnemyDirector* Director = GetDirector())
		{
			Director->UnregisterEnemy(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ANFEnemyBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() && IsAlive())
	{
		ServerThink(DeltaSeconds);
	}
}

bool ANFEnemyBase::IsAlive() const
{
	return HealthComponent && !HealthComponent->IsDead() && !bDeathHandled;
}

// ---------------------------------------------------------------------------------------------
// Visual state
// ---------------------------------------------------------------------------------------------

void ANFEnemyBase::SetVisualState(ENFEnemyVisualState NewState, uint8 AttackId)
{
	if (VisualState == NewState && VisualAttackId == AttackId)
	{
		return;
	}

	VisualState = NewState;
	VisualAttackId = AttackId;
	// The server does not get OnRep calls.
	NotifyVisualStateChanged();
}

void ANFEnemyBase::OnRep_VisualState()
{
	NotifyVisualStateChanged();
}

void ANFEnemyBase::NotifyVisualStateChanged()
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		RefreshVisuals();
	}
	OnVisualStateChanged.Broadcast(VisualState, VisualAttackId);
	K2_OnVisualStateChanged(VisualState, VisualAttackId);
}

void ANFEnemyBase::SetupHitVolume(UPrimitiveComponent* Component, bool bWeakPoint)
{
	if (!Component)
	{
		return;
	}

	Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Component->SetCollisionObjectType(ECC_WorldDynamic);
	Component->SetCollisionResponseToAllChannels(ECR_Ignore);
	Component->SetCollisionResponseToChannel(NF_ECC_Weapon, ECR_Block);
	Component->SetGenerateOverlapEvents(false);
	Component->SetCanEverAffectNavigation(false);
	if (bWeakPoint)
	{
		// The damage pipeline reads this tag as ENFHitZone::Head (the weapon's HeadshotMultiplier).
		Component->ComponentTags.Add(UNFHealthComponent::HeadTag);
	}
}

void ANFEnemyBase::SetupVisual(UStaticMeshComponent* Component)
{
	if (!Component)
	{
		return;
	}

	Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Component->SetGenerateOverlapEvents(false);
	Component->SetCanEverAffectNavigation(false);
}

void ANFEnemyBase::SetVisualColor(UMaterialInstanceDynamic* Material, const FLinearColor& Color)
{
	if (Material)
	{
		Material->SetVectorParameterValue(NFEnemy::ColorParameter, Color);
	}
}

// ---------------------------------------------------------------------------------------------
// Damage and death
// ---------------------------------------------------------------------------------------------

void ANFEnemyBase::HandleDamageTakenNative(const FNFDamageInfo& DamageInfo, float AppliedDamage)
{
	// OnDeath follows separately when this hit killed; reactions are for survivors.
	if (HealthComponent && HealthComponent->GetHealth() > 0.f && !bDeathHandled)
	{
		OnDamaged(DamageInfo, AppliedDamage);
	}
}

void ANFEnemyBase::HandleDeath(UNFHealthComponent* InHealthComponent, AController* Killer)
{
	if (bDeathHandled)
	{
		return;
	}
	bDeathHandled = true;

	// No more hits, no more blocking; the body stays visible for a moment.
	SetActorEnableCollision(false);

	if (HasAuthority())
	{
		if (UNFEnemyDirector* Director = GetDirector())
		{
			Director->ReleaseTokens(this);
		}
		StopMoving();
		GetCharacterMovement()->DisableMovement();
		OnDied();
		SetVisualState(ENFEnemyVisualState::Dead);
		SetLifeSpan(FMath::Max(CorpseSeconds, 0.1f));
	}
	else if (GetNetMode() != NM_DedicatedServer)
	{
		RefreshVisuals();
	}
}

float ANFEnemyBase::DealDamage(AActor* Victim, float Amount, ENFDamageType DamageType)
{
	UNFHealthComponent* VictimHealth = UNFHealthComponent::FindHealthComponent(Victim);
	if (!VictimHealth || !HasAuthority())
	{
		return 0.f;
	}

	FNFDamageInfo DamageInfo;
	DamageInfo.BaseDamage = Amount;
	DamageInfo.DamageType = DamageType;
	DamageInfo.HitLocation = Victim->GetActorLocation();
	DamageInfo.ShotDirection = (Victim->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	DamageInfo.Instigator = GetController();
	DamageInfo.Causer = this;
	return VictimHealth->ApplyDamage(DamageInfo);
}

void ANFEnemyBase::LaunchByDistance(ACharacter* Victim, const FVector& Direction, float DistanceCm)
{
	const UCharacterMovementComponent* Movement = Victim ? Victim->GetCharacterMovement() : nullptr;
	const FVector FlatDirection = Direction.GetSafeNormal2D();
	if (!Movement || DistanceCm <= 0.f || FlatDirection.IsNearlyZero())
	{
		return;
	}

	// Ballistic estimate: up at UpSpeed, airborne for 2 * UpSpeed / g, so the flat speed is distance / air time.
	// Ground friction after landing adds a little; treat DistanceCm as "about".
	const float UpSpeed = 200.f;
	const float Gravity = FMath::Max(FMath::Abs(Movement->GetGravityZ()), 1.f);
	const float AirTime = 2.f * UpSpeed / Gravity;
	const FVector LaunchVelocity = FlatDirection * (DistanceCm / AirTime) + FVector(0.f, 0.f, UpSpeed);
	Victim->LaunchCharacter(LaunchVelocity, /*bXYOverride*/ true, /*bZOverride*/ true);
}

void ANFEnemyBase::ApplyKnockback(const FVector& FromLocation, float DistanceCm)
{
	if (!HasAuthority() || !IsAlive() || DistanceCm <= 0.f)
	{
		return;
	}

	FVector Direction = (GetActorLocation() - FromLocation).GetSafeNormal2D();
	if (Direction.IsNearlyZero())
	{
		Direction = -GetActorForwardVector().GetSafeNormal2D();
	}

	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	const float Gravity = FMath::Max(FMath::Abs(Movement->GetGravityZ()), 1.f);
	const float AirTime = 2.f * KnockbackUpSpeed / Gravity;
	LaunchCharacter(Direction * (DistanceCm / AirTime) + FVector(0.f, 0.f, KnockbackUpSpeed), true, true);
}

// ---------------------------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------------------------

AAIController* ANFEnemyBase::GetAI() const
{
	return Cast<AAIController>(GetController());
}

UNFEnemyDirector* ANFEnemyBase::GetDirector() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetSubsystem<UNFEnemyDirector>() : nullptr;
}

float ANFEnemyBase::GetOwnWaterDepth() const
{
	return WetnessComponent ? WetnessComponent->GetWaterDepth() : -100000.f;
}

float ANFEnemyBase::GetWorldTime() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.f;
}

float ANFEnemyBase::GetDistance2DTo(const AActor* Other) const
{
	return Other ? static_cast<float>(FVector::Dist2D(GetActorLocation(), Other->GetActorLocation())) : TNumericLimits<float>::Max();
}

bool ANFEnemyBase::IsValidTarget(const APawn* Candidate) const
{
	if (!Candidate || !Candidate->IsPlayerControlled())
	{
		return false;
	}
	// Downed and hauled-out players are "dead" for the health component: neither faction attacks them.
	const UNFHealthComponent* CandidateHealth = UNFHealthComponent::FindHealthComponent(Candidate);
	return CandidateHealth && !CandidateHealth->IsDead();
}

APawn* ANFEnemyBase::FindNearestTarget() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	APawn* Best = nullptr;
	double BestDistance = TNumericLimits<double>::Max();
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		APawn* Candidate = PlayerController ? PlayerController->GetPawn() : nullptr;
		if (!IsValidTarget(Candidate))
		{
			continue;
		}
		const double Distance = FVector::DistSquared(GetActorLocation(), Candidate->GetActorLocation());
		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			Best = Candidate;
		}
	}
	return Best;
}

void ANFEnemyBase::UpdateTarget()
{
	const float Now = GetWorldTime();
	if (IsValidTarget(CurrentTarget.Get()) && Now < NextTargetSearchTime)
	{
		return;
	}

	NextTargetSearchTime = Now + NFEnemy::TargetSearchInterval;
	APawn* NewTarget = FindNearestTarget();
	if (NewTarget != CurrentTarget.Get())
	{
		CurrentTarget = NewTarget;
		if (AAIController* AI = GetAI())
		{
			if (NewTarget)
			{
				AI->SetFocus(NewTarget);
			}
			else
			{
				AI->ClearFocus(EAIFocusPriority::Gameplay);
			}
		}
	}
}

void ANFEnemyBase::MoveToTargetActor(AActor* Goal, float AcceptanceRadius)
{
	AAIController* AI = GetAI();
	if (AI && Goal)
	{
		AI->MoveToActor(Goal, AcceptanceRadius);
	}
}

void ANFEnemyBase::MoveToPoint(const FVector& Goal, float AcceptanceRadius)
{
	if (AAIController* AI = GetAI())
	{
		AI->MoveToLocation(Goal, AcceptanceRadius);
	}
}

void ANFEnemyBase::StopMoving()
{
	if (AAIController* AI = GetAI())
	{
		AI->StopMovement();
	}
}

bool ANFEnemyBase::IsMoving() const
{
	const AAIController* AI = GetAI();
	return AI && AI->GetMoveStatus() != EPathFollowingStatus::Idle;
}
