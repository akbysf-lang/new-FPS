#include "NFScuttler.h"
#include "NewFPS.h"
#include "NFEnemyData.h"
#include "NFEnemyDirector.h"
#include "NFHealthComponent.h"
#include "AIController.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

namespace NFScuttlerConst
{
	const FName AttackTokenPool(TEXT("ScuttlerMelee"));
	/** Starts asking for an attack token inside this distance (cm). */
	const float EngageDistance = 700.f;
	/** Seconds between move orders / token requests. */
	const float MoveOrderInterval = 0.4f;
	const float CircleOrderInterval = 0.8f;
	const float TokenRequestInterval = 0.5f;
	/** A Pinch still connects this far beyond PinchRange at the end of its telegraph (a step back avoids it). */
	const float PinchHitSlack = 20.f;
	/** Height difference (cm) beyond which a Pinch cannot connect. */
	const float PinchMaxHeightDifference = 120.f;

	// Grey-box colours. Drift reads as the darkest shape in the scene (style guide); Parched keeps the value
	// and loses saturation, the sac goes from turquoise to milk (spec 5.1).
	const FLinearColor ShellWet(0.02f, 0.05f, 0.06f);
	const FLinearColor ShellDry(0.05f, 0.06f, 0.06f);
	const FLinearColor ShellParched(0.06f, 0.06f, 0.06f);
	const FLinearColor SacLive(0.05f, 0.85f, 0.75f);
	const FLinearColor SacParched(0.75f, 0.78f, 0.76f);
	const FLinearColor Dead(0.01f, 0.01f, 0.01f);
	const FLinearColor Staggered(0.35f, 0.35f, 0.35f);
}

ANFScuttler::ANFScuttler()
{
	// Spec 5.1 (revision 2): radius 35 / half height 35, a 70 cm ball. Its top is well below a 110 cm low cover.
	GetCapsuleComponent()->InitCapsuleSize(35.f, 35.f);

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = 360.f;
	Movement->MaxAcceleration = 2400.f;
	Movement->RotationRate = FRotator(0.f, 720.f, 0.f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	// All offsets are relative to the capsule centre, which is 35 cm above the floor.
	// Shell: 72 long, 78 wide, 40 tall, from 10 to 50 cm above the floor.
	ShellHit = CreateDefaultSubobject<UBoxComponent>(TEXT("ShellHit"));
	ShellHit->SetupAttachment(GetCapsuleComponent());
	ShellHit->SetRelativeLocation(FVector(0.f, 0.f, -5.f));
	ShellHit->SetBoxExtent(FVector(36.f, 39.f, 20.f));
	SetupHitVolume(ShellHit, /*bWeakPoint*/ false);

	// Sac: 25 cm sphere on the back, its top at 67 cm: it clears the shell only from behind and above.
	SacHit = CreateDefaultSubobject<USphereComponent>(TEXT("SacHit"));
	SacHit->SetupAttachment(GetCapsuleComponent());
	SacHit->SetRelativeLocation(FVector(-14.f, 0.f, 20.f));
	SacHit->InitSphereRadius(12.5f);
	SetupHitVolume(SacHit, /*bWeakPoint*/ true);

	ClawHitLeft = CreateDefaultSubobject<UBoxComponent>(TEXT("ClawHitLeft"));
	ClawHitLeft->SetupAttachment(GetCapsuleComponent());
	ClawHitLeft->SetRelativeLocation(FVector(46.f, -25.f, -12.f));
	ClawHitLeft->SetBoxExtent(FVector(14.f, 9.f, 9.f));
	SetupHitVolume(ClawHitLeft, false);

	ClawHitRight = CreateDefaultSubobject<UBoxComponent>(TEXT("ClawHitRight"));
	ClawHitRight->SetupAttachment(GetCapsuleComponent());
	ClawHitRight->SetRelativeLocation(FVector(46.f, 25.f, -12.f));
	ClawHitRight->SetBoxExtent(FVector(14.f, 9.f, 9.f));
	SetupHitVolume(ClawHitRight, false);

	// Visuals: engine shapes are 100 cm, so scale = size / 100.
	ShellVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShellVisual"));
	ShellVisual->SetupAttachment(GetCapsuleComponent());
	ShellVisual->SetRelativeLocation(FVector(0.f, 0.f, -5.f));
	ShellVisual->SetRelativeScale3D(FVector(0.72f, 0.78f, 0.40f));
	SetupVisual(ShellVisual);

	SacVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SacVisual"));
	SacVisual->SetupAttachment(GetCapsuleComponent());
	SacVisual->SetRelativeLocation(FVector(-14.f, 0.f, 20.f));
	SacVisual->SetRelativeScale3D(FVector(0.25f));
	SetupVisual(SacVisual);

	ClawVisualLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ClawVisualLeft"));
	ClawVisualLeft->SetupAttachment(GetCapsuleComponent());
	ClawVisualLeft->SetRelativeLocation(FVector(46.f, -25.f, -12.f));
	ClawVisualLeft->SetRelativeScale3D(FVector(0.28f, 0.18f, 0.18f));
	SetupVisual(ClawVisualLeft);

	ClawVisualRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ClawVisualRight"));
	ClawVisualRight->SetupAttachment(GetCapsuleComponent());
	ClawVisualRight->SetRelativeLocation(FVector(46.f, 25.f, -12.f));
	ClawVisualRight->SetRelativeScale3D(FVector(0.28f, 0.18f, 0.18f));
	SetupVisual(ClawVisualRight);

	if (CubeMesh.Succeeded())
	{
		ShellVisual->SetStaticMesh(CubeMesh.Object);
		ClawVisualLeft->SetStaticMesh(CubeMesh.Object);
		ClawVisualRight->SetStaticMesh(CubeMesh.Object);
	}
	if (SphereMesh.Succeeded())
	{
		SacVisual->SetStaticMesh(SphereMesh.Object);
	}
}

void ANFScuttler::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANFScuttler, WaterState);
}

const UNFScuttlerData* ANFScuttler::GetData() const
{
	return Data ? Data.Get() : GetDefault<UNFScuttlerData>();
}

void ANFScuttler::BeginPlay()
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		ShellMaterial = ShellVisual->CreateAndSetMaterialInstanceDynamic(0);
		SacMaterial = SacVisual->CreateAndSetMaterialInstanceDynamic(0);
		ClawMaterialLeft = ClawVisualLeft->CreateAndSetMaterialInstanceDynamic(0);
		ClawMaterialRight = ClawVisualRight->CreateAndSetMaterialInstanceDynamic(0);
	}

	const UNFScuttlerData* Numbers = GetData();
	if (HasAuthority())
	{
		// Before Super: the health component's BeginPlay fills the health up to the maximum.
		HealthComponent->SetMaxHealth(Numbers->MaxHealth, /*bRefill*/ true);
		HealthComponent->ConfigureRegen(Numbers->WetRegenDelay, Numbers->WetRegenPerSecond, 0.f);
		CircleDirection = FMath::RandBool() ? 1.f : -1.f;
	}

	Super::BeginPlay();
}

// ---------------------------------------------------------------------------------------------
// Water: wet / dry / parched
// ---------------------------------------------------------------------------------------------

void ANFScuttler::UpdateWater(float DeltaSeconds)
{
	const UNFScuttlerData* Numbers = GetData();
	const bool bWet = GetOwnWaterDepth() >= Numbers->WetDepth;

	ENFScuttlerWaterState NewState = ENFScuttlerWaterState::Dry;
	if (bWet)
	{
		// Back in the water: parched is undone at once.
		DrySeconds = 0.f;
		NewState = ENFScuttlerWaterState::Wet;
	}
	else
	{
		DrySeconds += DeltaSeconds;
		if (DrySeconds >= Numbers->ParchedAfterSeconds)
		{
			NewState = ENFScuttlerWaterState::Parched;
		}
	}

	float Speed = Numbers->DrySpeed;
	if (NewState == ENFScuttlerWaterState::Wet)
	{
		Speed = Numbers->WetSpeed;
	}
	else if (NewState == ENFScuttlerWaterState::Parched)
	{
		Speed = Numbers->ParchedSpeed;
	}
	GetCharacterMovement()->MaxWalkSpeed = Speed;

	if (NewState != WaterState)
	{
		WaterState = NewState;
		HealthComponent->SetDamageTakenMultiplier(NewState == ENFScuttlerWaterState::Parched ? Numbers->ParchedDamageTakenMultiplier : 1.f);
		HealthComponent->SetRegenEnabled(NewState == ENFScuttlerWaterState::Wet);
		OnRep_WaterState();
	}
}

void ANFScuttler::OnRep_WaterState()
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		RefreshVisuals();
	}
}

// ---------------------------------------------------------------------------------------------
// State machine (server)
// ---------------------------------------------------------------------------------------------

void ANFScuttler::SetThinkState(EThinkState NewState)
{
	ThinkState = NewState;
	StateTime = 0.f;
}

bool ANFScuttler::TryTakeAttackToken(APawn* Target)
{
	if (bHasAttackToken)
	{
		return true;
	}

	const float Now = GetWorldTime();
	if (Now < NextTokenRequestTime)
	{
		return false;
	}
	NextTokenRequestTime = Now + NFScuttlerConst::TokenRequestInterval;

	UNFEnemyDirector* Director = GetDirector();
	bHasAttackToken = Director && Director->RequestToken(NFScuttlerConst::AttackTokenPool, Target, this, GetData()->AttackersPerTarget);
	return bHasAttackToken;
}

void ANFScuttler::ReleaseAttackToken()
{
	if (bHasAttackToken)
	{
		bHasAttackToken = false;
		if (UNFEnemyDirector* Director = GetDirector())
		{
			Director->ReleaseTokens(this);
		}
	}
}

void ANFScuttler::ServerThink(float DeltaSeconds)
{
	const UNFScuttlerData* Numbers = GetData();
	const float Now = GetWorldTime();

	UpdateWater(DeltaSeconds);
	StateTime += DeltaSeconds;

	// The leap finishes whatever happens to the target.
	if (ThinkState != EThinkState::LungeLeap)
	{
		const APawn* OldTarget = CurrentTarget.Get();
		UpdateTarget();
		if (OldTarget != CurrentTarget.Get())
		{
			// The token was for the old target.
			ReleaseAttackToken();
		}
	}

	APawn* Target = CurrentTarget.Get();
	if (!Target && ThinkState != EThinkState::LungeLeap)
	{
		if (ThinkState != EThinkState::Idle)
		{
			ReleaseAttackToken();
			StopMoving();
			SetThinkState(EThinkState::Idle);
			SetVisualState(ENFEnemyVisualState::Idle);
		}
		return;
	}

	const float Distance = Target ? GetDistance2DTo(Target) : 0.f;

	switch (ThinkState)
	{
	case EThinkState::Idle:
		SetThinkState(EThinkState::Chase);
		SetVisualState(ENFEnemyVisualState::Moving);
		break;

	case EThinkState::Chase:
		ThinkChase(Target, Distance);
		break;

	case EThinkState::Circle:
		ThinkCircle(Target, Distance);
		break;

	case EThinkState::PinchTelegraph:
		if (StateTime >= Numbers->PinchTelegraph)
		{
			// A step back during the telegraph gets the player out of reach.
			const float HeightDifference = FMath::Abs(static_cast<float>(Target->GetActorLocation().Z - GetActorLocation().Z));
			if (IsValidTarget(Target)
				&& Distance <= Numbers->PinchRange + NFScuttlerConst::PinchHitSlack
				&& HeightDifference <= NFScuttlerConst::PinchMaxHeightDifference)
			{
				DealDamage(Target, Numbers->PinchDamage, ENFDamageType::EnemyMelee);
			}
			RecoverDuration = Numbers->PinchRecovery;
			SetThinkState(EThinkState::PinchRecover);
			SetVisualState(ENFEnemyVisualState::Recovering, AttackPinch);
		}
		break;

	case EThinkState::PinchRecover:
	case EThinkState::LungeRecover:
		if (StateTime >= RecoverDuration)
		{
			// Hand the token back so the ones circling get their turn.
			ReleaseAttackToken();
			NextAttackTime = Now + Numbers->AttackCooldown;
			SetThinkState(EThinkState::Chase);
			SetVisualState(ENFEnemyVisualState::Moving);
		}
		break;

	case EThinkState::LungeTelegraph:
		if (StateTime >= Numbers->LungeTelegraph)
		{
			// Leap at where the player is now: a side step during the flight makes it miss.
			const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
			const float LeapTime = FMath::Max(Numbers->LungeLeapTime, 0.05f);
			const float Gravity = FMath::Max(FMath::Abs(GetCharacterMovement()->GetGravityZ()), 1.f);
			FVector LaunchVelocity = FVector(ToTarget.X, ToTarget.Y, 0.f) / LeapTime;
			// Up speed that brings it back to the same height after LeapTime.
			LaunchVelocity.Z = 0.5f * Gravity * LeapTime;
			LaunchCharacter(LaunchVelocity, true, true);

			bLungeHit = false;
			SetThinkState(EThinkState::LungeLeap);
			SetVisualState(ENFEnemyVisualState::Attacking, AttackLunge);
		}
		break;

	case EThinkState::LungeLeap:
		if (!bLungeHit && IsValidTarget(Target)
			&& FVector::DistSquared(Target->GetActorLocation(), GetActorLocation()) <= FMath::Square(Numbers->LungeHitRadius))
		{
			bLungeHit = true;
			DealDamage(Target, Numbers->LungeDamage, ENFDamageType::EnemyMelee);
		}
		if (StateTime >= Numbers->LungeLeapTime)
		{
			// A miss leaves it exposed.
			RecoverDuration = bLungeHit ? Numbers->LungeHitRecovery : Numbers->LungeMissRecovery;
			SetThinkState(EThinkState::LungeRecover);
			SetVisualState(bLungeHit ? ENFEnemyVisualState::Recovering : ENFEnemyVisualState::Vulnerable, AttackLunge);
		}
		break;

	case EThinkState::Stagger:
		if (StateTime >= Numbers->StaggerSeconds)
		{
			SetThinkState(EThinkState::Chase);
			SetVisualState(ENFEnemyVisualState::Moving);
		}
		break;
	}
}

void ANFScuttler::ThinkChase(APawn* Target, float Distance)
{
	const UNFScuttlerData* Numbers = GetData();
	const float Now = GetWorldTime();

	// Close enough to matter: ask for one of the attack slots on this player.
	if (Distance <= NFScuttlerConst::EngageDistance && !TryTakeAttackToken(Target))
	{
		if (Distance <= Numbers->CircleMaxRadius + 100.f)
		{
			SetThinkState(EThinkState::Circle);
			NextMoveOrderTime = 0.f;
			return;
		}
	}

	if (bHasAttackToken && Now >= NextAttackTime)
	{
		if (Distance <= Numbers->PinchRange)
		{
			StopMoving();
			SetThinkState(EThinkState::PinchTelegraph);
			SetVisualState(ENFEnemyVisualState::Telegraph, AttackPinch);
			return;
		}

		// Lunge: only wet, only from mid range, only with a clear line.
		const AAIController* AI = GetAI();
		if (WaterState == ENFScuttlerWaterState::Wet
			&& Distance >= Numbers->LungeMinRange && Distance <= Numbers->LungeMaxRange
			&& AI && AI->LineOfSightTo(Target))
		{
			StopMoving();
			SetThinkState(EThinkState::LungeTelegraph);
			SetVisualState(ENFEnemyVisualState::Telegraph, AttackLunge);
			return;
		}
	}

	if (Now >= NextMoveOrderTime)
	{
		NextMoveOrderTime = Now + NFScuttlerConst::MoveOrderInterval;
		MoveToTargetActor(Target, Numbers->PinchRange * 0.6f);
	}
}

void ANFScuttler::ThinkCircle(APawn* Target, float Distance)
{
	const UNFScuttlerData* Numbers = GetData();
	const float Now = GetWorldTime();

	// A slot came free, or the player ran off: back to the chase.
	if (TryTakeAttackToken(Target) || Distance > NFScuttlerConst::EngageDistance)
	{
		SetThinkState(EThinkState::Chase);
		NextMoveOrderTime = 0.f;
		return;
	}

	if (Now >= NextMoveOrderTime)
	{
		NextMoveOrderTime = Now + NFScuttlerConst::CircleOrderInterval;

		// Walk a ring around the player, 250-400 cm out, one arc at a time.
		const FVector TargetLocation = Target->GetActorLocation();
		FVector FromTarget = (GetActorLocation() - TargetLocation).GetSafeNormal2D();
		if (FromTarget.IsNearlyZero())
		{
			FromTarget = FVector::ForwardVector;
		}
		const float Radius = FMath::FRandRange(Numbers->CircleMinRadius, Numbers->CircleMaxRadius);
		const FVector RingPoint = TargetLocation + FromTarget.RotateAngleAxis(45.f * CircleDirection, FVector::UpVector) * Radius;
		MoveToPoint(RingPoint, 40.f);
	}
}

void ANFScuttler::OnDamaged(const FNFDamageInfo& DamageInfo, float AppliedDamage)
{
	const UNFScuttlerData* Numbers = GetData();

	// Kedge and heavy shots shove it; the push comes from whoever hit it.
	const AActor* Source = DamageInfo.Causer.Get();
	const FVector PushFrom = Source ? Source->GetActorLocation() : GetActorLocation() - DamageInfo.ShotDirection * 100.f;
	if (DamageInfo.DamageType == ENFDamageType::Melee)
	{
		ApplyKnockback(PushFrom, Numbers->MeleeKnockback);
	}
	else if (DamageInfo.DamageType == ENFDamageType::Bullet && AppliedDamage >= Numbers->HeavyHitDamage)
	{
		ApplyKnockback(PushFrom, Numbers->HeavyHitKnockback);
	}

	// Every hit it survives stops it for a moment and cancels a wind-up. A leap already in the air goes on.
	if (ThinkState != EThinkState::LungeLeap && Numbers->StaggerSeconds > 0.f)
	{
		ReleaseAttackToken();
		StopMoving();
		SetThinkState(EThinkState::Stagger);
		SetVisualState(ENFEnemyVisualState::Staggered);
	}
}

// ---------------------------------------------------------------------------------------------
// Grey-box presentation
// ---------------------------------------------------------------------------------------------

void ANFScuttler::RefreshVisuals()
{
	const ENFEnemyVisualState State = GetVisualState();
	const int32 AttackId = GetVisualAttackId();

	FLinearColor ShellColor = NFScuttlerConst::ShellDry;
	FLinearColor SacColor = NFScuttlerConst::SacLive;
	if (WaterState == ENFScuttlerWaterState::Wet)
	{
		ShellColor = NFScuttlerConst::ShellWet;
	}
	else if (WaterState == ENFScuttlerWaterState::Parched)
	{
		ShellColor = NFScuttlerConst::ShellParched;
		SacColor = NFScuttlerConst::SacParched;
	}

	if (State == ENFEnemyVisualState::Dead)
	{
		ShellColor = NFScuttlerConst::Dead;
		SacColor = NFScuttlerConst::Dead;
	}
	else if (State == ENFEnemyVisualState::Staggered)
	{
		ShellColor = NFScuttlerConst::Staggered;
	}

	SetVisualColor(ShellMaterial, ShellColor);
	SetVisualColor(SacMaterial, SacColor);
	SetVisualColor(ClawMaterialLeft, ShellColor);
	SetVisualColor(ClawMaterialRight, ShellColor);

	// Two telegraph poses (spec 5.1): Pinch = claws up, Lunge = body down.
	const bool bPinchTelegraph = State == ENFEnemyVisualState::Telegraph && AttackId == AttackPinch;
	const bool bLungeTelegraph = State == ENFEnemyVisualState::Telegraph && AttackId == AttackLunge;
	const float ClawZ = bPinchTelegraph ? 22.f : -12.f;
	ClawVisualLeft->SetRelativeLocation(FVector(46.f, -25.f, ClawZ));
	ClawVisualRight->SetRelativeLocation(FVector(46.f, 25.f, ClawZ));
	ShellVisual->SetRelativeScale3D(FVector(0.72f, 0.78f, (bLungeTelegraph || State == ENFEnemyVisualState::Dead) ? 0.22f : 0.40f));
}
