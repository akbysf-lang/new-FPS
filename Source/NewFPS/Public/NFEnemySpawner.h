#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/NetSerialization.h"
#include "GameFramework/Actor.h"
#include "Templates/SubclassOf.h"
#include "NFEnemySpawner.generated.h"

class ANFEnemyBase;
class USceneComponent;

UENUM(BlueprintType)
enum class ENFEnemyFaction : uint8
{
	/** Scuttlers: come with the water. */
	Drift,
	/** Attendants: hold the dry ground. */
	Wardens
};

/** When a spawn point may be used, relative to the tide (prototype-spec.md 7.5). */
UENUM(BlueprintType)
enum class ENFSpawnWaterRule : uint8
{
	Always,
	/** Only while the water is at or above the point (minus the point's margin). Drift points. */
	Submerged,
	/** Only while the water is below the point. Warden points on the Half level. */
	Dry
};

/**
 * One place enemies can come from. Put it on the floor (the actor origin is the floor height) and set the
 * faction, the water rule and, if a wave should use only some points, a group name.
 *
 * Spec 7.5 mapping: D-Low at the sea edge = Drift / Always; D-Low under the pier = Drift / Submerged;
 * D-Half and D-High = Drift / Submerged with SubmergedMargin 50; W-Half = Wardens / Dry; W-High = Wardens / Always.
 */
UCLASS()
class NEWFPS_API ANFEnemySpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	ANFEnemySpawnPoint();

	/** The water rule alone (distance and line of sight are the spawner's business). */
	bool IsUsableAtWaterZ(float WaterZ, bool bHasTide) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	ENFEnemyFaction Faction = ENFEnemyFaction::Drift;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	ENFSpawnWaterRule WaterRule = ENFSpawnWaterRule::Submerged;

	/** Submerged rule: the point already counts this many cm before the water reaches it (drains and gutters). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0.0", Units = "cm"))
	float SubmergedMargin = 0.f;

	/** Optional. A wave with the same PointGroup uses only these points (e.g. "W-High"). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	FName Group;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;
};

/** One line of the wave script. */
USTRUCT(BlueprintType)
struct FNFWaveEntry
{
	GENERATED_BODY()

	/** Tide clock time (seconds) the wave is due. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "0.0", Units = "s"))
	float TimeSeconds = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	ENFEnemyFaction Faction = ENFEnemyFaction::Drift;

	/** Size for one player; scaled by the player count multiplier and cut by the alive cap. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave", meta = (ClampMin = "1"))
	int32 SoloCount = 2;

	/** Optional: only spawn points with this Group. None = any point of the faction whose water rule allows it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wave")
	FName PointGroup;
};

/**
 * The wave script and the spawn rules (prototype-spec.md 5.3, 7.5, 8.2).
 * Create: Data Asset -> NFWaveData, assign on the level's ANFEnemySpawner.
 * The class defaults are the spec's solo script for the 12 minute cycle and are used when no asset is assigned.
 */
UCLASS(BlueprintType)
class NEWFPS_API UNFWaveData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UNFWaveData();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(FPrimaryAssetType(TEXT("NFWaves")), GetFName());
	}

	/** M(N) = 1 + PlayerScaleSlope * (N - 1). */
	float GetPlayerMultiplier(int32 NumPlayers) const;

	int32 GetAliveCap(ENFEnemyFaction Faction, int32 NumPlayers) const;

	TSubclassOf<ANFEnemyBase> GetEnemyClass(ENFEnemyFaction Faction) const;

	/** In time order. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Waves")
	TArray<FNFWaveEntry> Waves;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classes")
	TSubclassOf<ANFEnemyBase> DriftClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Classes")
	TSubclassOf<ANFEnemyBase> WardenClass;

	/** Enemy count grows with the players; enemy health and damage never do. Spec: 0.6 -> x1.0 / 1.6 / 2.2 / 2.8. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scaling", meta = (ClampMin = "0.0"))
	float PlayerScaleSlope = 0.6f;

	/** Living Scuttlers allowed at once, by player count (index 0 = solo). Spec: 6 / 10 / 13 / 17. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scaling")
	TArray<int32> DriftAliveCapByPlayerCount;

	/** Living Attendants allowed at once, by player count. Spec: 3 / 5 / 7 / 8. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scaling")
	TArray<int32> WardenAliveCapByPlayerCount;

	/** A spawn point must be at least this far from every standing player... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules", meta = (ClampMin = "0.0", Units = "cm"))
	float MinPlayerDistance = 1500.f;

	/** ...and, for Warden points, out of every standing player's sight (shutters and hotel doors). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules")
	bool bRequireOutOfSight = true;

	/**
	 * The same sight rule for Drift points. Off (spec 7.5, revision 3): the sea edge points lie on the open beach
	 * and are nearly always in view, so with the rule on the water line waves silently fail to spawn. Scuttlers
	 * coming out of the water in plain sight is the fantasy; the 1.5 s foam telegraph covers the pop-in.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules")
	bool bDriftRequireOutOfSight = false;

	/** No usable point: the wave waits this long and tries again... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules", meta = (ClampMin = "0.5", Units = "s"))
	float RetryDelay = 5.f;

	/**
	 * ...this many times, then it is skipped and logged. Spec 7.5: 3 (3 x 5 s = 15 s; more than the 20 s between
	 * flood waves would stack a late wave on the next one).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules", meta = (ClampMin = "0"))
	int32 MaxRetries = 3;

	/** Warning before the enemies appear (foam / shutter noise). OnSpawnTelegraph fires at its start. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules", meta = (ClampMin = "0.0", Units = "s"))
	float SpawnTelegraphSeconds = 1.5f;

	/** Enemies of one wave are scattered this far around their point. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rules", meta = (ClampMin = "0.0", Units = "cm"))
	float SpawnScatterRadius = 150.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNFOnSpawnTelegraph, FVector, Location, ENFEnemyFaction, Faction);

/**
 * Runs the wave script against the tide clock. Place exactly one in the level, plus ANFEnemySpawnPoint actors
 * and a NavMeshBoundsVolume. Server only; clients just see the spawned enemies.
 *
 * Per wave: count = round(SoloCount x M(players)), at least 1, cut down to what the alive cap leaves; a wave that
 * meets a full cap is skipped (not queued). Points must match the faction (and group), satisfy their water rule,
 * and be far from the players - Warden points also out of their sight; if none qualifies the wave is retried a
 * few times and then skipped. Every skipped wave writes a "Telemetry WaveSkipped" line to LogNewFPS.
 */
UCLASS()
class NEWFPS_API ANFEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	ANFEnemySpawner();

	/** Never null: falls back to the UNFWaveData class defaults. */
	UFUNCTION(BlueprintPure, Category = "Spawner")
	const UNFWaveData* GetWaveData() const;

	/** Server only. Off = no new waves (enemies already alive stay). */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SetSpawningEnabled(bool bEnabled);

	/** Server only: forget pending waves and continue the script from this tide clock time (waves before it are skipped). */
	void AuthResetToTime(float ElapsedSeconds);

	/**
	 * Server only: put Count enemies of a faction on the NavMesh between MinDistance and MaxDistance of Center,
	 * right now, ignoring spawn points, scaling and the telegraph (Flood Drill). Returns how many were spawned.
	 */
	int32 AuthSpawnGroupAround(ENFEnemyFaction Faction, int32 Count, const FVector& Center, float MinDistance, float MaxDistance);

	/** Server: waves skipped since the script was last (re)started because no spawn point was usable (spec 1.5). */
	UFUNCTION(BlueprintPure, Category = "Spawner")
	int32 GetSkippedWaveCount() const { return SkippedNoPointCount; }

	/** Every machine: enemies are about to appear at Location (SpawnTelegraphSeconds later). Hook the telegraph sound/VFX here. */
	UPROPERTY(BlueprintAssignable, Category = "Spawner")
	FNFOnSpawnTelegraph OnSpawnTelegraph;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastSpawnTelegraph(const FVector_NetQuantize& Location, ENFEnemyFaction Faction);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Wave script and rules. Leave empty to use the class defaults of UNFWaveData (the spec's script). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TObjectPtr<UNFWaveData> WaveData;

	/** Start running the script in BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	bool bAutoStart = true;

private:
	struct FPendingWave
	{
		int32 WaveIndex = 0;
		float NextTryTime = 0.f;
		int32 Retries = 0;
	};

	enum class EWaveResult : uint8
	{
		Done,
		Retry
	};

	void PollWaves();
	float GetScriptTime() const;
	EWaveResult TrySpawnWave(const FNFWaveEntry& Wave);
	void GatherUsablePoints(const FNFWaveEntry& Wave, TArray<ANFEnemySpawnPoint*>& OutPoints) const;
	/** Telemetry (spec 1.5): count and log a wave that did not spawn. bCapFull = by design (5.3), else no usable point. */
	void NoteWaveSkipped(const FNFWaveEntry& Wave, bool bCapFull);
	void SpawnAtPoint(TWeakObjectPtr<ANFEnemySpawnPoint> Point, ENFEnemyFaction Faction, int32 Count);
	ANFEnemyBase* SpawnEnemy(TSubclassOf<ANFEnemyBase> EnemyClass, const FVector& FloorLocation, const FRotator& Rotation);

	bool bSpawningEnabled = false;
	int32 NextWaveIndex = 0;
	TArray<FPendingWave> PendingWaves;
	int32 SkippedNoPointCount = 0;
	int32 SkippedCapFullCount = 0;
	FTimerHandle PollTimerHandle;
};
