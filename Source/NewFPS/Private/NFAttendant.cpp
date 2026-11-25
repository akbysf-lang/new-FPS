#include "NFAttendant.h"
#include "NewFPS.h"
#include "NFEnemyData.h"
#include "NFEnemyDirector.h"
#include "NFEnemyProjectile.h"
#include "NFHealthComponent.h"
#include "NFTideManager.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace NFAttendantConst
{
	const FName FireTokenPool(TEXT("AttendantFire"));
	const float MoveOrderInterval = 0.5f;
	const float WaterCheckInterval = 1.f;
	/** NavMesh samples taken when looking for higher ground or a new firing spot. */
	const int32 NavSamples = 8;
	/** Higher ground has to be at least this much higher to be worth the walk (cm). */
	const float MinRetreatGain = 100.f;
	/** Gives up a retreat walk after this long. */
	const float MaxRetreatSeconds = 12.f;
	/** Half period of the 1 Hz service blink. */
	const float LampBlinkHalfPeriod = 0.5f;

	const FLinearColor BodyColor(0.80f, 0.74f, 0.58f);
	const FLinearColor BodySparking(0.55f, 0.70f, 0.95f);
	const FLinearColor BodyStaggered(1.f, 1.f, 1.f);
	const FLinearColor Dead(0.03f, 0.03f, 0.03f);
}

ANFAttendant::ANFAttendant()
{
	// Spec 5.2: radius 40, half height 95 (190 tall).
	GetCapsuleComponent()->InitCapsuleSize(40.f, 95.f);

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->MaxWalkSpeed = 300.f;
	Movement->RotationRate = FRotator(0.f, 360.f, 0.f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	// All offsets are relative to the capsule centre, which is 95 cm above the floor.
	// Body: from the floor up to 154 cm, where the lamp sphere starts.
	BodyHit = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BodyHit"));
	BodyHit->SetupAttachment(GetCapsuleComponent());
	BodyHit->SetRelativeLocation(FVector(0.f, 0.f, -18.f));
	BodyHit->InitCapsuleSize(32.f, 77.f);
	SetupHitVolume(BodyHit, /*bWeakPoint*/ false);

	// Lamp: radius 18, centre at 172 cm (95 + 77): spans 154-190 and stays inside the capsule top (revision 2).
	LampHit = CreateDefaultSubobject<USphereComponent>(TEXT("LampHit"));
	LampHit->SetupAttachment(GetCapsuleComponent());
	LampHit->SetRelativeLocation(FVector(0.f, 0.f, 77.f));
	LampHit->InitSphereRadius(18.f);
	SetupHitVolume(LampHit, /*bWeakPoint*/ true);

	// Visuals: engine shapes are 100 cm, so scale = size / 100. Thin and upright: the opposite of a Scuttler.
	BodyVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyVisual"));
	BodyVisual->SetupAttachment(GetCapsuleComponent());
	BodyVisual->SetRelativeLocation(FVector(0.f, 0.f, -18.f));
	BodyVisual->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.54f));
	SetupVisual(BodyVisual);

	// The visible lamp sits a little higher than the hit sphere's centre (175 cm) and is 26 cm across.
	LampVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LampVisual"));
	LampVisual->SetupAttachment(GetCapsuleComponent());
	LampVisual->SetRelativeLocation(FVector(6.f, 0.f, 80.f));
	LampVisual->SetRelativeScale3D(FVector(0.26f));
	SetupVisual(LampVisual);

	if (CylinderMesh.Succeeded())
	{
		BodyVisual->SetStaticMesh(CylinderMesh.Object);
	}
	if (SphereMesh.Succeeded())
	{
		LampVisual->SetStaticMesh(SphereMesh.Object);
	}
}

void ANFAttendant::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANFAttendant, bSparking);
}

const UNFAttendantData* ANFAttendant::GetData() const
{
	return Data ? Data.Get() : GetDefault<UNFAttendantData>();
}

void ANFAttendant::BeginPlay()
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		BodyMaterial = BodyVisual->CreateAndSetMaterialInstanceDynamic(0);
		LampMaterial = LampVisual->CreateAndSetMaterialInstanceDynamic(0);
	}

	if (HasAuthority())
	{
		// Before Super: the health component's BeginPlay fills the health up to the maximum.
		HealthComponent->SetMaxHealth(GetData()->MaxHealth, /*bRefill*/ true);
		MissSide = FMath::RandBool() ? 1.f : -1.f;
	}

	Super::BeginPlay();
}

void ANFAttendant::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(LampBlinkTimerHandle);

	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------------------------
// Water
// ---------------------------------------------------------------------------------------------

bool ANFAttendant::UpdateWater(float DeltaSeconds)
{
	const UNFAttendantData* Numbers = GetData();
	const float Depth = GetOwnWaterDepth();
	const float Now = GetWorldTime();

	// Waist deep for 2 s without a break: it shorts out. This is what pushing it into the water is for.
	if (Depth >= Numbers->ShortCircuitDepth)
	{
		WaistSeconds += DeltaSeconds;
		if (WaistSeconds >= Numbers->ShortCircuitSeconds)
		{
			UE_LOG(LogNewFPS, Verbose, TEXT("%s shorted out"), *GetName());
			HealthComponent->Kill(nullptr);
			return false;
		}
	}
	else
	{
		WaistSeconds = 0.f;
	}

	const bool bNowSparking = Depth >= Numbers->KneeDepth;
	if (bNowSparking != bSparking)
	{
		bSparking = bNowSparking;
		OnRep_Sparking();
	}

	// Is the tide coming in? Sampled once a second.
	if (Now >= NextWaterCheckTime)
	{
		NextWaterCheckTime = Now + NFAttendantConst::WaterCheckInterval;
		if (const ANFTideManager* TideManager = ANFTideManager::Get(GetWorld()))
		{
			const float WaterZ = TideManager->GetWaterZAt(GetActorLocation());
			bWaterRising = WaterZ > LastWaterZ + 0.5f;
			LastWaterZ = WaterZ;
		}
		else
		{
			bWaterRising = false;
		}
	}

	const bool bWalking = ThinkState == EThinkState::Idle;
	GetCharacterMovement()->MaxWalkSpeed = (bWalking ? Numbers->PatrolSpeed : Numbers->RepositionSpeed)
		* (bSparking ? Numbers->KneeSpeedMultiplier : 1.f);
	return true;
}

void ANFAttendant::OnRep_Sparking()
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		RefreshVisuals();
	}
}

bool ANFAttendant::TryStartRetreat()
{
	const UNFAttendantData* Numbers = GetData();

	// Rising water within 150 cm of its feet: go up. The depth is negative while the water is below the feet.
	if (!bWaterRising || GetOwnWaterDepth() < -Numbers->RetreatWaterMargin)
	{
		return false;
	}

	UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!Navigation)
	{
		return false;
	}

	// No authored retreat points in the prototype: sample the NavMesh and take the highest reachable spot.
	const FVector Origin = GetActorLocation();
	FVector BestPoint = Origin;
	float BestZ = static_cast<float>(Origin.Z) + NFAttendantConst::MinRetreatGain;
	bool bFound = false;
	for (int32 Sample = 0; Sample < NFAttendantConst::NavSamples; ++Sample)
	{
		FNavLocation Candidate;
		if (Navigation->GetRandomReachablePointInRadius(Origin, Numbers->RetreatSearchRadius, Candidate) && Candidate.Location.Z > BestZ)
		{
			BestZ = static_cast<float>(Candidate.Location.Z);
			BestPoint = Candidate.Location;
			bFound = true;
		}
	}

	if (!bFound)
	{
		return false;
	}

	// It turns its back on the player while it walks: that is the reward (spec 5.2).
	ReleaseFireToken();
	if (AAIController* AI = GetAI())
	{
		AI->SetFocalPoint(BestPoint);
	}
	MoveToPoint(BestPoint, 60.f);
	SetThinkState(EThinkState::Retreat);
	SetVisualState(ENFEnemyVisualState::Moving);
	return true;
}

// ---------------------------------------------------------------------------------------------
// State machine (server)
// ---------------------------------------------------------------------------------------------

void ANFAttendant::SetThinkState(EThinkState NewState)
{
	ThinkState = NewState;
	StateTime = 0.f;
}

void ANFAttendant::ReleaseFireToken()
{
	if (bHasFireToken)
	{
		bHasFireToken = false;
		if (UNFEnemyDirector* Director = GetDirector())
		{
			Director->ReleaseTokens(this);
		}
	}
}

void ANFAttendant::ServerThink(float DeltaSeconds)
{
	const UNFAttendantData* Numbers = GetData();
	const float Now = GetWorldTime();

	if (!UpdateWater(DeltaSeconds))
	{
		return;
	}
	StateTime += DeltaSeconds;

	const bool bHadTarget = CurrentTarget.IsValid();
	UpdateTarget();
	APawn* Target = CurrentTarget.Get();

	if (!Target)
	{
		// Nobody to shoot at (everyone down, or nobody near). A retreat walk still finishes.
		if (ThinkState != EThinkState::Idle && ThinkState != EThinkState::Retreat)
		{
			ReleaseFireToken();
			StopMoving();
			SetThinkState(EThinkState::Idle);
			SetVisualState(ENFEnemyVisualState::Idle);
		}
		if (ThinkState == EThinkState::Idle)
		{
			return;
		}
	}
	else if (!bHadTarget)
	{
		// Just noticed a player: the first burst is a warning shot.
		bFirstBurstPending = true;
	}

	const float Distance = Target ? GetDistance2DTo(Target) : 0.f;

	switch (ThinkState)
	{
	case EThinkState::Idle:
		NextRepositionTime = Now + FMath::FRandRange(Numbers->RepositionIntervalMin, Numbers->RepositionIntervalMax);
		SetThinkState(EThinkState::Engage);
		SetVisualState(ENFEnemyVisualState::Moving);
		break;

	case EThinkState::Engage:
		ThinkEngage(Target, Distance);
		break;

	case EThinkState::BurstTelegraph:
		if (StateTime >= Numbers->BurstTelegraph)
		{
			ShotsFiredInBurst = 0;
			NextShotTime = Now;
			SetThinkState(EThinkState::BurstFiring);
			SetVisualState(ENFEnemyVisualState::Attacking, AttackPunchBurst);
		}
		break;

	case EThinkState::BurstFiring:
		while (ShotsFiredInBurst < Numbers->BurstShots && Now >= NextShotTime)
		{
			FireProjectile(Target, bFirstBurstPending);
			++ShotsFiredInBurst;
			NextShotTime += Numbers->BurstShotInterval;
		}
		if (ShotsFiredInBurst >= Numbers->BurstShots)
		{
			bFirstBurstPending = false;
			++BurstsSinceService;
			ReleaseFireToken();
			NextBurstTime = Now + Numbers->BurstCooldown;

			if (BurstsSinceService >= Numbers->BurstsPerService)
			{
				// Service window: exposed, lamp blinking, weak point still live.
				BurstsSinceService = 0;
				StopMoving();
				SetThinkState(EThinkState::Service);
				SetVisualState(ENFEnemyVisualState::Vulnerable);
			}
			else
			{
				SetThinkState(EThinkState::Engage);
				SetVisualState(ENFEnemyVisualState::Moving);
			}
		}
		break;

	case EThinkState::Service:
		if (StateTime >= Numbers->ServiceSeconds)
		{
			SetThinkState(EThinkState::Engage);
			SetVisualState(ENFEnemyVisualState::Moving);
		}
		break;

	case EThinkState::ShoveTelegraph:
		if (StateTime >= Numbers->ShoveTelegraph)
		{
			if (IsValidTarget(Target) && Distance <= Numbers->ShoveRange + Numbers->ShoveHitSlack)
			{
				DealDamage(Target, Numbers->ShoveDamage, ENFDamageType::EnemyMelee);
				// Can send the player down a ramp or into the water.
				LaunchByDistance(Cast<ACharacter>(Target), Target->GetActorLocation() - GetActorLocation(), Numbers->ShovePush);
			}
			SetThinkState(EThinkState::ShoveRecover);
			SetVisualState(ENFEnemyVisualState::Recovering, AttackShove);
		}
		break;

	case EThinkState::ShoveRecover:
		// Neither fires nor moves: the answer window for a player who dodged or was pushed.
		if (StateTime >= Numbers->ShoveRecovery)
		{
			NextShoveTime = Now + Numbers->ShoveCooldown;
			SetThinkState(EThinkState::Engage);
			SetVisualState(ENFEnemyVisualState::Moving);
		}
		break;

	case EThinkState::Stagger:
		if (StateTime >= Numbers->StaggerSeconds)
		{
			SetThinkState(EThinkState::Engage);
			SetVisualState(ENFEnemyVisualState::Moving);
		}
		break;

	case EThinkState::Retreat:
		if (!IsMoving() || StateTime >= NFAttendantConst::MaxRetreatSeconds)
		{
			if (AAIController* AI = GetAI())
			{
				// Back to looking at the player (or at nothing).
				if (Target)
				{
					AI->SetFocus(Target);
				}
				else
				{
					AI->ClearFocus(EAIFocusPriority::Gameplay);
				}
			}
			SetThinkState(Target ? EThinkState::Engage : EThinkState::Idle);
			SetVisualState(Target ? ENFEnemyVisualState::Moving : ENFEnemyVisualState::Idle);
		}
		break;
	}
}

void ANFAttendant::ThinkEngage(APawn* Target, float Distance)
{
	const UNFAttendantData* Numbers = GetData();
	const float Now = GetWorldTime();
	AAIController* AI = GetAI();

	// A retreat walk looks at its destination; make sure it is looking at the player again.
	if (AI && AI->GetFocusActor() != Target)
	{
		AI->SetFocus(Target);
	}

	// 1. A player in its face gets shoved.
	if (Distance <= Numbers->ShoveRange && Now >= NextShoveTime)
	{
		ReleaseFireToken();
		StopMoving();
		SetThinkState(EThinkState::ShoveTelegraph);
		SetVisualState(ENFEnemyVisualState::Telegraph, AttackShove);
		return;
	}

	// 2. The tide comes first.
	if (TryStartRetreat())
	{
		return;
	}

	const bool bLineOfSight = AI && AI->LineOfSightTo(Target);

	// 3. Fire when it can: in range, sees the player, cooldown over, and one of the fire tokens is free.
	if (bLineOfSight && Distance <= Numbers->MaxRange && Now >= NextBurstTime)
	{
		UNFEnemyDirector* Director = GetDirector();
		const TArray<int32>& TokenTable = Numbers->FireTokensByPlayerCount;
		int32 MaxFiring = 2;
		if (Director && TokenTable.Num() > 0)
		{
			MaxFiring = TokenTable[FMath::Clamp(Director->GetPlayerCount() - 1, 0, TokenTable.Num() - 1)];
		}

		bHasFireToken = Director && Director->RequestToken(NFAttendantConst::FireTokenPool, nullptr, this, MaxFiring);
		if (bHasFireToken)
		{
			// The telegraph always starts with the lamp in the player's view (line of sight was just checked).
			StopMoving();
			SetThinkState(EThinkState::BurstTelegraph);
			SetVisualState(ENFEnemyVisualState::Telegraph, AttackPunchBurst);
			return;
		}
		// No token: fall through and keep moving / waiting.
	}

	// 4. Keep the player inside the preferred band and in view.
	if (Now >= NextMoveOrderTime)
	{
		NextMoveOrderTime = Now + NFAttendantConst::MoveOrderInterval;

		if (!bLineOfSight || Distance > Numbers->PreferredMaxRange)
		{
			// Close in to the middle of the band (MoveTo stops at the acceptance radius).
			MoveToTargetActor(Target, (Numbers->PreferredMinRange + Numbers->PreferredMaxRange) * 0.5f);
		}
		else if (Now >= NextRepositionTime && !IsMoving())
		{
			NextRepositionTime = Now + FMath::FRandRange(Numbers->RepositionIntervalMin, Numbers->RepositionIntervalMax);
			TryReposition(Target);
		}
	}
}

void ANFAttendant::TryReposition(APawn* Target)
{
	const UNFAttendantData* Numbers = GetData();
	UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!Navigation || !Target)
	{
		return;
	}

	// Stand-in for "changes cover every 6-10 s": another reachable spot that keeps the player in the preferred band.
	const FVector TargetLocation = Target->GetActorLocation();
	for (int32 Sample = 0; Sample < NFAttendantConst::NavSamples; ++Sample)
	{
		FNavLocation Candidate;
		if (!Navigation->GetRandomReachablePointInRadius(GetActorLocation(), Numbers->RepositionRadius, Candidate))
		{
			continue;
		}
		const float CandidateDistance = static_cast<float>(FVector::Dist2D(Candidate.Location, TargetLocation));
		if (CandidateDistance >= Numbers->PreferredMinRange && CandidateDistance <= Numbers->PreferredMaxRange)
		{
			MoveToPoint(Candidate.Location, 50.f);
			return;
		}
	}
}

void ANFAttendant::FireProjectile(APawn* Target, bool bMissOnPurpose)
{
	UWorld* World = GetWorld();
	const UNFAttendantData* Numbers = GetData();
	if (!World || !Numbers->ProjectileClass)
	{
		return;
	}

	const FVector SpawnLocation = GetActorLocation() + GetActorRotation().RotateVector(MuzzleOffset);

	// No lead: aimed at where the player is now, so a strafing player walks out of it.
	FVector AimDirection = GetActorForwardVector();
	if (Target)
	{
		AimDirection = (Target->GetActorLocation() - SpawnLocation).GetSafeNormal();
	}
	if (bMissOnPurpose)
	{
		// "Looks clever": the first burst goes wide and gives the player time to find cover.
		AimDirection = AimDirection.RotateAngleAxis(Numbers->FirstBurstMissAngle * MissSide, FVector::UpVector);
	}
	if (Numbers->ProjectileSpread > 0.f)
	{
		AimDirection = FMath::VRandCone(AimDirection, FMath::DegreesToRadians(Numbers->ProjectileSpread));
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.Instigator = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (ANFEnemyProjectile* Projectile = World->SpawnActor<ANFEnemyProjectile>(Numbers->ProjectileClass, SpawnLocation, AimDirection.Rotation(), SpawnParameters))
	{
		Projectile->InitProjectile(Numbers->ProjectileDamage, Numbers->ProjectileSpeed);
	}
}

void ANFAttendant::OnDamaged(const FNFDamageInfo& DamageInfo, float AppliedDamage)
{
	const UNFAttendantData* Numbers = GetData();
	const AActor* Source = DamageInfo.Causer.Get();
	const FVector PushFrom = Source ? Source->GetActorLocation() : GetActorLocation() - DamageInfo.ShotDirection * 100.f;

	if (AppliedDamage >= Numbers->StaggerDamage)
	{
		// A big single hit (Purser on the lamp, most of a Gaffer shell): cuts the telegraph or the burst short
		// and knocks it back - towards the water, if the player picked the angle.
		ReleaseFireToken();
		StopMoving();
		ApplyKnockback(PushFrom, Numbers->StaggerKnockback);
		SetThinkState(EThinkState::Stagger);
		SetVisualState(ENFEnemyVisualState::Staggered);
	}
	else if (DamageInfo.DamageType == ENFDamageType::Melee)
	{
		// The Kedge pushes without interrupting.
		ApplyKnockback(PushFrom, Numbers->MeleeKnockback);
	}
}

// ---------------------------------------------------------------------------------------------
// Grey-box presentation
// ---------------------------------------------------------------------------------------------

void ANFAttendant::RefreshVisuals()
{
	const ENFEnemyVisualState State = GetVisualState();
	const int32 AttackId = GetVisualAttackId();
	FTimerManager& TimerManager = GetWorldTimerManager();

	FLinearColor Body = bSparking ? NFAttendantConst::BodySparking : NFAttendantConst::BodyColor;
	if (State == ENFEnemyVisualState::Dead)
	{
		Body = NFAttendantConst::Dead;
	}
	else if (State == ENFEnemyVisualState::Staggered)
	{
		Body = NFAttendantConst::BodyStaggered;
	}
	SetVisualColor(BodyMaterial, Body);

	// Lamp language (spec 5.2): red = fire incoming, slow blink in the calm colour = being serviced.
	// It never goes dark while the Attendant works, because it is also the weak point.
	if (State == ENFEnemyVisualState::Vulnerable)
	{
		if (!TimerManager.IsTimerActive(LampBlinkTimerHandle))
		{
			bLampBlinkOn = false;
			BlinkLamp();
			TimerManager.SetTimer(LampBlinkTimerHandle, this, &ANFAttendant::BlinkLamp, NFAttendantConst::LampBlinkHalfPeriod, true);
		}
		return;
	}

	TimerManager.ClearTimer(LampBlinkTimerHandle);

	FLinearColor Lamp = LampCalmColor;
	if (State == ENFEnemyVisualState::Dead)
	{
		Lamp = NFAttendantConst::Dead;
	}
	else if ((State == ENFEnemyVisualState::Telegraph || State == ENFEnemyVisualState::Attacking) && AttackId == AttackPunchBurst)
	{
		Lamp = LampAlertColor;
	}
	SetVisualColor(LampMaterial, Lamp);
}

void ANFAttendant::BlinkLamp()
{
	// Low intensity, never off.
	bLampBlinkOn = !bLampBlinkOn;
	SetVisualColor(LampMaterial, bLampBlinkOn ? LampCalmColor : LampCalmColor * 0.35f);
}
