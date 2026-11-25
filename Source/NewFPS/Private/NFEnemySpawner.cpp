#include "NFEnemySpawner.h"
#include "NewFPS.h"
#include "NFAttendant.h"
#include "NFEnemyBase.h"
#include "NFEnemyDirector.h"
#include "NFHealthComponent.h"
#include "NFScuttler.h"
#include "NFTideManager.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "NavigationSystem.h"
#include "TimerManager.h"

namespace NFSpawnerConst
{
	/** Seconds between looks at the wave script. */
	const float PollInterval = 0.25f;
	/** Sight test target above a spawn point's floor (cm): roughly a Scuttler's back. */
	const float SightCheckHeight = 60.f;
	/** Attempts per enemy to find a NavMesh spot in the requested ring (AuthSpawnGroupAround). */
	const int32 RingSamples = 12;
}

// ---------------------------------------------------------------------------------------------
// ANFEnemySpawnPoint
// ---------------------------------------------------------------------------------------------

ANFEnemySpawnPoint::ANFEnemySpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
}

bool ANFEnemySpawnPoint::IsUsableAtWaterZ(float WaterZ, bool bHasTide) const
{
	const float PointZ = static_cast<float>(GetActorLocation().Z);
	switch (WaterRule)
	{
	case ENFSpawnWaterRule::Submerged:
		// "Comes with the water": no tide in the level means no water to come with.
		return bHasTide && WaterZ >= PointZ - SubmergedMargin;
	case ENFSpawnWaterRule::Dry:
		return !bHasTide || WaterZ < PointZ;
	default:
		return true;
	}
}

// ---------------------------------------------------------------------------------------------
// UNFWaveData: prototype-spec.md 8.2 (solo numbers)
// ---------------------------------------------------------------------------------------------

UNFWaveData::UNFWaveData()
{
	DriftClass = ANFScuttler::StaticClass();
	WardenClass = ANFAttendant::StaticClass();

	DriftAliveCapByPlayerCount = { 6, 10, 13, 17 };
	WardenAliveCapByPlayerCount = { 3, 5, 7, 8 };

	auto AddWave = [this](float Minutes, float Seconds, ENFEnemyFaction Faction, int32 SoloCount, const TCHAR* PointGroup = nullptr)
	{
		FNFWaveEntry Wave;
		Wave.TimeSeconds = Minutes * 60.f + Seconds;
		Wave.Faction = Faction;
		Wave.SoloCount = SoloCount;
		Wave.PointGroup = PointGroup ? FName(PointGroup) : NAME_None;
		Waves.Add(Wave);
	};

	const ENFEnemyFaction Drift = ENFEnemyFaction::Drift;
	const ENFEnemyFaction Wardens = ENFEnemyFaction::Wardens;

	// Ebb: the first kills on the terrace, then the first Attendants.
	AddWave(0, 5, Drift, 2);
	AddWave(0, 20, Wardens, 2, TEXT("W-High"));
	AddWave(1, 0, Wardens, 2, TEXT("W-Half"));
	AddWave(1, 0, Drift, 3);
	// Slack low: Scuttlers from the sea, Attendants come down.
	AddWave(2, 20, Drift, 2);
	AddWave(2, 30, Wardens, 2);
	AddWave(3, 0, Drift, 2);
	AddWave(3, 30, Wardens, 2);
	AddWave(3, 40, Drift, 2);
	AddWave(4, 20, Drift, 2);
	// 5:00 the bell: the water line arrives with six of them.
	AddWave(5, 0, Drift, 6);
	AddWave(5, 10, Wardens, 2);
	AddWave(5, 20, Drift, 3);
	AddWave(5, 40, Drift, 3);
	AddWave(6, 0, Drift, 3);
	AddWave(6, 20, Drift, 3);
	// 6:30-7:00 half tide pause: nothing.
	AddWave(7, 5, Wardens, 3);
	AddWave(7, 15, Drift, 3);
	AddWave(7, 35, Drift, 3);
	AddWave(7, 55, Drift, 3);
	AddWave(8, 15, Drift, 3);
	AddWave(8, 35, Drift, 3);
	AddWave(8, 50, Drift, 2);
	AddWave(9, 0, Wardens, 2);
	AddWave(9, 10, Drift, 2);
	AddWave(9, 30, Drift, 2);
	AddWave(9, 50, Drift, 2);
	// Slack high: winch defence, every 20 s until 11:40.
	AddWave(10, 0, Drift, 3);
	AddWave(10, 20, Drift, 3);
	AddWave(10, 40, Drift, 3);
	AddWave(11, 0, Drift, 3);
	AddWave(11, 20, Drift, 3);
	AddWave(11, 40, Drift, 3);
}

float UNFWaveData::GetPlayerMultiplier(int32 NumPlayers) const
{
	return 1.f + PlayerScaleSlope * static_cast<float>(FMath::Max(NumPlayers, 1) - 1);
}

int32 UNFWaveData::GetAliveCap(ENFEnemyFaction Faction, int32 NumPlayers) const
{
	const TArray<int32>& Caps = Faction == ENFEnemyFaction::Drift ? DriftAliveCapByPlayerCount : WardenAliveCapByPlayerCount;
	if (Caps.Num() == 0)
	{
		return MAX_int32;
	}
	return Caps[FMath::Clamp(NumPlayers - 1, 0, Caps.Num() - 1)];
}

TSubclassOf<ANFEnemyBase> UNFWaveData::GetEnemyClass(ENFEnemyFaction Faction) const
{
	return Faction == ENFEnemyFaction::Drift ? DriftClass : WardenClass;
}

// ---------------------------------------------------------------------------------------------
// ANFEnemySpawner
// ---------------------------------------------------------------------------------------------

ANFEnemySpawner::ANFEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	// Replicated only so the spawn telegraph can reach clients.
	bReplicates = true;
	bAlwaysRelevant = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
}

const UNFWaveData* ANFEnemySpawner::GetWaveData() const
{
	return WaveData ? WaveData.Get() : GetDefault<UNFWaveData>();
}

void ANFEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	// Waves that were due before the script starts (a tide that starts mid-cycle) are not replayed.
	AuthResetToTime(GetScriptTime());
	bSpawningEnabled = bAutoStart;
	GetWorldTimerManager().SetTimer(PollTimerHandle, this, &ANFEnemySpawner::PollWaves, NFSpawnerConst::PollInterval, true);
}

void ANFEnemySpawner::SetSpawningEnabled(bool bEnabled)
{
	if (HasAuthority())
	{
		bSpawningEnabled = bEnabled;
		if (!bEnabled)
		{
			PendingWaves.Reset();
		}
	}
}

float ANFEnemySpawner::GetScriptTime() const
{
	// The script runs on the tide clock, so a paused or moved tide takes the waves with it.
	if (const ANFTideManager* TideManager = ANFTideManager::Get(GetWorld()))
	{
		return static_cast<float>(TideManager->GetElapsedSeconds());
	}
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.f;
}

void ANFEnemySpawner::AuthResetToTime(float ElapsedSeconds)
{
	if (!HasAuthority())
	{
		return;
	}

	const TArray<FNFWaveEntry>& Waves = GetWaveData()->Waves;
	PendingWaves.Reset();
	NextWaveIndex = 0;
	while (NextWaveIndex < Waves.Num() && Waves[NextWaveIndex].TimeSeconds < ElapsedSeconds)
	{
		++NextWaveIndex;
	}
}

void ANFEnemySpawner::PollWaves()
{
	UWorld* World = GetWorld();
	if (!bSpawningEnabled || !World)
	{
		return;
	}

	const UNFWaveData* Script = GetWaveData();
	const TArray<FNFWaveEntry>& Waves = Script->Waves;
	const float ScriptTime = GetScriptTime();
	const float Now = World->GetTimeSeconds();

	while (NextWaveIndex < Waves.Num() && Waves[NextWaveIndex].TimeSeconds <= ScriptTime)
	{
		FPendingWave Pending;
		Pending.WaveIndex = NextWaveIndex;
		Pending.NextTryTime = Now;
		PendingWaves.Add(Pending);
		++NextWaveIndex;
	}

	for (int32 Index = PendingWaves.Num() - 1; Index >= 0; --Index)
	{
		FPendingWave& Pending = PendingWaves[Index];
		if (Now < Pending.NextTryTime)
		{
			continue;
		}

		if (!Waves.IsValidIndex(Pending.WaveIndex) || TrySpawnWave(Waves[Pending.WaveIndex]) == EWaveResult::Done)
		{
			PendingWaves.RemoveAt(Index);
		}
		else if (++Pending.Retries > Script->MaxRetries)
		{
			UE_LOG(LogNewFPS, Log, TEXT("Wave at %.0f s dropped: no usable spawn point."), Waves[Pending.WaveIndex].TimeSeconds);
			PendingWaves.RemoveAt(Index);
		}
		else
		{
			Pending.NextTryTime = Now + Script->RetryDelay;
		}
	}
}

void ANFEnemySpawner::GatherUsablePoints(const FNFWaveEntry& Wave, TArray<ANFEnemySpawnPoint*>& OutPoints) const
{
	UWorld* World = GetWorld();
	const UNFWaveData* Script = GetWaveData();
	const ANFTideManager* TideManager = ANFTideManager::Get(World);

	// Standing players only: nobody has to be surprised on behalf of a downed one.
	TArray<FVector, TInlineAllocator<4>> PlayerEyes;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		const APlayerController* PlayerController = It->Get();
		const APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
		const UNFHealthComponent* Health = UNFHealthComponent::FindHealthComponent(Pawn);
		if (Pawn && (!Health || !Health->IsDead()))
		{
			PlayerEyes.Add(Pawn->GetPawnViewLocation());
		}
	}

	FCollisionQueryParams SightParams(SCENE_QUERY_STAT(NFSpawnSight), /*bTraceComplex*/ false);
	const FCollisionObjectQueryParams StaticWorld(ECC_WorldStatic);

	for (TActorIterator<ANFEnemySpawnPoint> It(World); It; ++It)
	{
		ANFEnemySpawnPoint* Point = *It;
		if (Point->Faction != Wave.Faction || (!Wave.PointGroup.IsNone() && Point->Group != Wave.PointGroup))
		{
			continue;
		}

		const FVector PointLocation = Point->GetActorLocation();
		const float WaterZ = TideManager ? TideManager->GetWaterZAt(PointLocation) : 0.f;
		if (!Point->IsUsableAtWaterZ(WaterZ, TideManager != nullptr))
		{
			continue;
		}

		bool bAllowed = true;
		const FVector SightTarget = PointLocation + FVector(0.f, 0.f, NFSpawnerConst::SightCheckHeight);
		for (const FVector& Eye : PlayerEyes)
		{
			if (FVector::DistSquared(Eye, PointLocation) < FMath::Square(Script->MinPlayerDistance))
			{
				bAllowed = false;
				break;
			}
			// Nothing solid in between = the player can see the point.
			if (Script->bRequireOutOfSight && !World->LineTraceTestByObjectType(Eye, SightTarget, StaticWorld, SightParams))
			{
				bAllowed = false;
				break;
			}
		}

		if (bAllowed)
		{
			OutPoints.Add(Point);
		}
	}
}

ANFEnemySpawner::EWaveResult ANFEnemySpawner::TrySpawnWave(const FNFWaveEntry& Wave)
{
	UWorld* World = GetWorld();
	const UNFWaveData* Script = GetWaveData();
	UNFEnemyDirector* Director = World->GetSubsystem<UNFEnemyDirector>();
	const TSubclassOf<ANFEnemyBase> EnemyClass = Script->GetEnemyClass(Wave.Faction);
	if (!Director || !EnemyClass)
	{
		return EWaveResult::Done;
	}

	// Count scales with the players; health and damage never do (spec 5.3).
	const int32 NumPlayers = Director->GetPlayerCount();
	const int32 Wanted = FMath::Max(FMath::RoundToInt(static_cast<float>(Wave.SoloCount) * Script->GetPlayerMultiplier(NumPlayers)), 1);
	const int32 Room = Script->GetAliveCap(Wave.Faction, NumPlayers) - Director->CountAlive(EnemyClass);
	if (Room <= 0)
	{
		// A wave that meets a full cap is skipped, not queued.
		return EWaveResult::Done;
	}
	int32 Remaining = FMath::Min(Wanted, Room);

	TArray<ANFEnemySpawnPoint*> Points;
	GatherUsablePoints(Wave, Points);
	if (Points.Num() == 0)
	{
		return EWaveResult::Retry;
	}

	// Shuffle, then spread the wave over the points.
	for (int32 Index = Points.Num() - 1; Index > 0; --Index)
	{
		Points.Swap(Index, FMath::RandRange(0, Index));
	}

	const int32 PointsUsed = FMath::Min(Points.Num(), Remaining);
	const float TelegraphSeconds = FMath::Max(Script->SpawnTelegraphSeconds, 0.f);
	for (int32 Index = 0; Index < PointsUsed; ++Index)
	{
		// Ceiling division: earlier points take the remainder.
		const int32 CountHere = (Remaining + (PointsUsed - Index) - 1) / (PointsUsed - Index);
		Remaining -= CountHere;

		ANFEnemySpawnPoint* Point = Points[Index];
		MulticastSpawnTelegraph(Point->GetActorLocation(), Wave.Faction);

		if (TelegraphSeconds <= 0.f)
		{
			SpawnAtPoint(Point, Wave.Faction, CountHere);
		}
		else
		{
			FTimerHandle SpawnTimerHandle;
			GetWorldTimerManager().SetTimer(
				SpawnTimerHandle,
				FTimerDelegate::CreateUObject(this, &ANFEnemySpawner::SpawnAtPoint, TWeakObjectPtr<ANFEnemySpawnPoint>(Point), Wave.Faction, CountHere),
				TelegraphSeconds,
				false);
		}
	}

	return EWaveResult::Done;
}

void ANFEnemySpawner::MulticastSpawnTelegraph_Implementation(const FVector_NetQuantize& Location, ENFEnemyFaction Faction)
{
	OnSpawnTelegraph.Broadcast(Location, Faction);
}

void ANFEnemySpawner::SpawnAtPoint(TWeakObjectPtr<ANFEnemySpawnPoint> Point, ENFEnemyFaction Faction, int32 Count)
{
	const ANFEnemySpawnPoint* SpawnPoint = Point.Get();
	if (!SpawnPoint || !bSpawningEnabled)
	{
		return;
	}

	const UNFWaveData* Script = GetWaveData();
	const TSubclassOf<ANFEnemyBase> EnemyClass = Script->GetEnemyClass(Faction);
	const FVector Center = SpawnPoint->GetActorLocation();
	const FRotator Rotation(0.f, SpawnPoint->GetActorRotation().Yaw, 0.f);

	for (int32 Index = 0; Index < Count; ++Index)
	{
		// The first one stands on the point; the rest are scattered so they do not spawn inside each other.
		FVector Offset = FVector::ZeroVector;
		if (Index > 0)
		{
			const FVector2D Scatter = FMath::RandPointInCircle(Script->SpawnScatterRadius);
			Offset = FVector(Scatter.X, Scatter.Y, 0.f);
		}
		SpawnEnemy(EnemyClass, Center + Offset, Rotation);
	}
}

ANFEnemyBase* ANFEnemySpawner::SpawnEnemy(TSubclassOf<ANFEnemyBase> EnemyClass, const FVector& FloorLocation, const FRotator& Rotation)
{
	UWorld* World = GetWorld();
	if (!World || !EnemyClass)
	{
		return nullptr;
	}

	// Spawn points and NavMesh points are floor heights; a character's origin is its capsule centre.
	float HalfHeight = 50.f;
	if (const ACharacter* DefaultCharacter = Cast<ACharacter>(EnemyClass->GetDefaultObject()))
	{
		HalfHeight = DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	return World->SpawnActor<ANFEnemyBase>(EnemyClass, FloorLocation + FVector(0.f, 0.f, HalfHeight + 2.f), Rotation, SpawnParameters);
}

int32 ANFEnemySpawner::AuthSpawnGroupAround(ENFEnemyFaction Faction, int32 Count, const FVector& Center, float MinDistance, float MaxDistance)
{
	UWorld* World = GetWorld();
	if (!HasAuthority() || !World || Count <= 0)
	{
		return 0;
	}

	const TSubclassOf<ANFEnemyBase> EnemyClass = GetWaveData()->GetEnemyClass(Faction);
	UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!EnemyClass || !Navigation)
	{
		UE_LOG(LogNewFPS, Warning, TEXT("AuthSpawnGroupAround: no enemy class or no navigation system (is there a NavMeshBoundsVolume?)."));
		return 0;
	}

	int32 Spawned = 0;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		for (int32 Sample = 0; Sample < NFSpawnerConst::RingSamples; ++Sample)
		{
			FNavLocation Candidate;
			if (!Navigation->GetRandomReachablePointInRadius(Center, MaxDistance, Candidate))
			{
				continue;
			}
			// The last sample is taken whatever its distance, so a cramped NavMesh still gets its enemies.
			if (FVector::Dist2D(Candidate.Location, Center) < MinDistance && Sample < NFSpawnerConst::RingSamples - 1)
			{
				continue;
			}

			const FRotator FaceCenter(0.f, (Center - Candidate.Location).Rotation().Yaw, 0.f);
			if (SpawnEnemy(EnemyClass, Candidate.Location, FaceCenter))
			{
				++Spawned;
			}
			break;
		}
	}
	return Spawned;
}
