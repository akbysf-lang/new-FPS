#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Templates/SubclassOf.h"
#include "NFEnemyData.generated.h"

class ANFEnemyProjectile;

/**
 * Scuttler numbers (prototype-spec.md 5.1). Create: Data Asset -> NFScuttlerData, assign on a Scuttler Blueprint.
 * The class defaults are the spec's starting values and are used when the Scuttler has no asset.
 * The capsule (35 / 35) and the hit volumes are on the ANFScuttler class itself.
 */
UCLASS(BlueprintType)
class NEWFPS_API UNFScuttlerData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(FPrimaryAssetType(TEXT("NFEnemy")), GetFName());
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 60.f;

	// ---- Water ----

	/** Wet when the water stands this deep at its own feet (the 30 cm on the terrace counts). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Water", meta = (Units = "cm"))
	float WetDepth = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "0.0"))
	float WetSpeed = 504.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "0.0"))
	float DrySpeed = 360.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "0.0"))
	float ParchedSpeed = 300.f;

	/** Unbroken seconds out of the water until it is Parched. Getting wet resets it at once. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "0.0", Units = "s"))
	float ParchedAfterSeconds = 20.f;

	/** Damage taken multiplier while Parched (Purser body 75 and Kedge 60 then kill in one). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "0.0"))
	float ParchedDamageTakenMultiplier = 1.5f;

	/** Wet only: seconds without damage before it heals. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "0.0", Units = "s"))
	float WetRegenDelay = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "0.0"))
	float WetRegenPerSecond = 6.f;

	// ---- Pinch ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinch", meta = (ClampMin = "0.0", Units = "cm"))
	float PinchRange = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinch", meta = (ClampMin = "0.0", Units = "s"))
	float PinchTelegraph = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinch", meta = (ClampMin = "0.0"))
	float PinchDamage = 12.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinch", meta = (ClampMin = "0.0", Units = "s"))
	float PinchRecovery = 0.8f;

	/** Wait after any attack's recovery before the next attack. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pinch", meta = (ClampMin = "0.0", Units = "s"))
	float AttackCooldown = 1.5f;

	// ---- Lunge (wet only) ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lunge", meta = (ClampMin = "0.0", Units = "cm"))
	float LungeMinRange = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lunge", meta = (ClampMin = "0.0", Units = "cm"))
	float LungeMaxRange = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lunge", meta = (ClampMin = "0.0", Units = "s"))
	float LungeTelegraph = 0.6f;

	/** Flight time of the leap. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lunge", meta = (ClampMin = "0.05", Units = "s"))
	float LungeLeapTime = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lunge", meta = (ClampMin = "0.0"))
	float LungeDamage = 18.f;

	/** The leap connects when it passes this close to the target (centre to centre). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lunge", meta = (ClampMin = "0.0", Units = "cm"))
	float LungeHitRadius = 95.f;

	/** Exposed this long after a leap that missed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lunge", meta = (ClampMin = "0.0", Units = "s"))
	float LungeMissRecovery = 1.f;

	/** Recovery after a leap that hit. Not in the spec; developer placeholder. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Lunge", meta = (ClampMin = "0.0", Units = "s"))
	float LungeHitRecovery = 0.4f;

	// ---- Crowd ----

	/** Scuttlers that may attack the same player at once; the others circle. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crowd", meta = (ClampMin = "1"))
	int32 AttackersPerTarget = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crowd", meta = (ClampMin = "0.0", Units = "cm"))
	float CircleMinRadius = 250.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crowd", meta = (ClampMin = "0.0", Units = "cm"))
	float CircleMaxRadius = 400.f;

	// ---- Reactions ----

	/** Every hit it survives stops it for this long. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactions", meta = (ClampMin = "0.0", Units = "s"))
	float StaggerSeconds = 0.3f;

	/** Pushed this far by a Kedge hit. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactions", meta = (ClampMin = "0.0", Units = "cm"))
	float MeleeKnockback = 200.f;

	/**
	 * "The Gaffer pushes" (spec 5.1) has no number. Placeholder: one shot that applies at least
	 * HeavyHitDamage pushes it HeavyHitKnockback cm. 45 = three Gaffer pellets; a Purser body shot (50) also counts.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactions", meta = (ClampMin = "0.0"))
	float HeavyHitDamage = 45.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactions", meta = (ClampMin = "0.0", Units = "cm"))
	float HeavyHitKnockback = 120.f;
};

/**
 * Attendant numbers (prototype-spec.md 5.2). Create: Data Asset -> NFAttendantData, assign on an Attendant Blueprint.
 * The class defaults are the spec's starting values and are used when the Attendant has no asset.
 * The capsule (40 / 95) and the lamp hit sphere (radius 18 at Z 172) are on the ANFAttendant class itself.
 */
UCLASS(BlueprintType)
class NEWFPS_API UNFAttendantData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UNFAttendantData();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(FPrimaryAssetType(TEXT("NFEnemy")), GetFName());
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 150.f;

	// ---- Movement and range ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float PatrolSpeed = 300.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0"))
	float RepositionSpeed = 420.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0", Units = "cm"))
	float PreferredMinRange = 1200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0", Units = "cm"))
	float PreferredMaxRange = 2500.f;

	/** Does not open fire beyond this. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0", Units = "cm"))
	float MaxRange = 3500.f;

	/** Moves to another spot every Min..Max seconds. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0", Units = "s"))
	float RepositionIntervalMin = 6.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0", Units = "s"))
	float RepositionIntervalMax = 10.f;

	/** How far a reposition goes. Not in the spec; developer placeholder. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "0.0", Units = "cm"))
	float RepositionRadius = 800.f;

	// ---- Punch Burst ----

	/** Lamp red + "click-clack" before the first projectile. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Punch Burst", meta = (ClampMin = "0.0", Units = "s"))
	float BurstTelegraph = 0.7f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Punch Burst", meta = (ClampMin = "1"))
	int32 BurstShots = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Punch Burst", meta = (ClampMin = "0.01", Units = "s"))
	float BurstShotInterval = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Punch Burst", meta = (ClampMin = "0.0"))
	float ProjectileDamage = 10.f;

	/** Slow on purpose: at 25 m the flight takes 0.7 s, a strafing player walks out of it. No lead. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Punch Burst", meta = (ClampMin = "1.0"))
	float ProjectileSpeed = 3500.f;

	/** Half-angle of the aim error in degrees. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Punch Burst", meta = (ClampMin = "0.0"))
	float ProjectileSpread = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Punch Burst", meta = (ClampMin = "0.0", Units = "s"))
	float BurstCooldown = 2.f;

	/**
	 * The first burst after noticing a player misses on purpose: aimed this many degrees to one side.
	 * The angle is not in the spec; developer placeholder (6 degrees = 1.3 m off at 12 m).
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Punch Burst", meta = (ClampMin = "0.0"))
	float FirstBurstMissAngle = 6.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Punch Burst")
	TSubclassOf<ANFEnemyProjectile> ProjectileClass;

	/** Attendants allowed to fire at once, by player count (index 0 = solo). Spec: 2 / 3 / 4 / 4. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Punch Burst")
	TArray<int32> FireTokensByPlayerCount;

	// ---- Service window ----

	/** After this many bursts it stops to be serviced. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Service", meta = (ClampMin = "1"))
	int32 BurstsPerService = 4;

	/** Stands exposed this long; the lamp blinks slowly and stays a weak point. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Service", meta = (ClampMin = "0.0", Units = "s"))
	float ServiceSeconds = 3.f;

	// ---- Shove ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shove", meta = (ClampMin = "0.0", Units = "cm"))
	float ShoveRange = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shove", meta = (ClampMin = "0.0", Units = "s"))
	float ShoveTelegraph = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shove", meta = (ClampMin = "0.0"))
	float ShoveDamage = 8.f;

	/** The player is pushed about this far. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shove", meta = (ClampMin = "0.0", Units = "cm"))
	float ShovePush = 300.f;

	/** Neither fires nor moves for this long after the shove. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shove", meta = (ClampMin = "0.0", Units = "s"))
	float ShoveRecovery = 0.5f;

	/** The shove still connects this far beyond ShoveRange at the end of the telegraph (cm). Developer placeholder. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shove", meta = (ClampMin = "0.0", Units = "cm"))
	float ShoveHitSlack = 40.f;

	/** Wait before the next shove. Not in the spec; developer placeholder. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shove", meta = (ClampMin = "0.0", Units = "s"))
	float ShoveCooldown = 1.5f;

	// ---- Reactions ----

	/** One shot that applies at least this much staggers it (Purser weak point, 6+ Gaffer pellets). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactions", meta = (ClampMin = "0.0"))
	float StaggerDamage = 80.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactions", meta = (ClampMin = "0.0", Units = "s"))
	float StaggerSeconds = 0.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactions", meta = (ClampMin = "0.0", Units = "cm"))
	float StaggerKnockback = 150.f;

	/** Pushed this far by a Kedge hit. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Reactions", meta = (ClampMin = "0.0", Units = "cm"))
	float MeleeKnockback = 100.f;

	// ---- Water ----

	/** Slowed (and sparking) from this depth at its own feet. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Water", meta = (Units = "cm"))
	float KneeDepth = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float KneeSpeedMultiplier = 0.7f;

	/** Shorts out after standing this deep... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Water", meta = (Units = "cm"))
	float ShortCircuitDepth = 100.f;

	/** ...for this long without a break. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "0.0", Units = "s"))
	float ShortCircuitSeconds = 2.f;

	/** Heads for higher ground when rising water is within this far below its feet. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Water", meta = (Units = "cm"))
	float RetreatWaterMargin = 150.f;

	/** How far it looks for higher ground. Not in the spec; developer placeholder. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Water", meta = (ClampMin = "0.0", Units = "cm"))
	float RetreatSearchRadius = 3000.f;
};
