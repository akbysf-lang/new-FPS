#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NFRulesData.generated.h"

/**
 * Designer-tunable player and mission rules (prototype-spec.md 2.3, 6 and 8.3): health, downed/revive, solo
 * spare line, strongbox handling, quota and the Flood Drill start.
 * Create in the editor: Content Browser -> Miscellaneous -> Data Asset -> NFRulesData, then assign the same asset
 * to BP_NFCharacter (NewFPS | Rules) and BP_NFGameMode (NewFPS | Rules).
 * The class defaults are the spec's starting values and are used when no asset is assigned.
 */
UCLASS(BlueprintType)
class NEWFPS_API UNFRulesData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(FPrimaryAssetType(TEXT("NFRules")), GetFName());
	}

	// ---- Health (6.1) ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	/** Seconds without damage before health starts to come back. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "0.0", Units = "s"))
	float RegenDelay = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "0.0"))
	float RegenPerSecond = 8.f;

	/** Regeneration stops here; the rest needs a first aid box. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "0.0"))
	float RegenCap = 50.f;

	// ---- Downed and revive (6.2) ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Downed", meta = (ClampMin = "1.0", Units = "s"))
	float BleedoutSeconds = 60.f;

	/** The bleed-out clock runs this much faster while the downed player lies in water of WetBleedoutDepth or more. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Downed", meta = (ClampMin = "1.0"))
	float WetBleedoutMultiplier = 3.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Downed", meta = (ClampMin = "0.0", Units = "cm"))
	float WetBleedoutDepth = 50.f;

	/** A teammate holds Interact this long (damage does not interrupt; letting go resets). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Downed", meta = (ClampMin = "0.1", Units = "s"))
	float ReviveHoldSeconds = 3.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Downed", meta = (ClampMin = "1.0"))
	float ReviveHealth = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Downed", meta = (ClampMin = "0.0", Units = "s"))
	float ReviveInvulnerableSeconds = 2.f;

	/** Bled out: the lifeline brings the player back at a player start after this long... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Downed", meta = (ClampMin = "0.1", Units = "s"))
	float LifelineRespawnDelay = 45.f;

	/** ...with this much health. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Downed", meta = (ClampMin = "1.0"))
	float LifelineRespawnHealth = 50.f;

	// ---- Solo spare line (6.3): once per mission ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Solo", meta = (ClampMin = "0.0", Units = "s"))
	float SpareLineDelay = 3.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Solo", meta = (ClampMin = "1.0"))
	float SpareLineHealth = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Solo", meta = (ClampMin = "0.0", Units = "s"))
	float SpareLineInvulnerableSeconds = 2.f;

	/** Enemies inside this radius are pushed away when the spare line pulls the player up. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Solo", meta = (ClampMin = "0.0", Units = "cm"))
	float SpareLineKnockbackRadius = 400.f;

	/** How far they are pushed. The spec only gives the radius; this number is a developer placeholder. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Solo", meta = (ClampMin = "0.0", Units = "cm"))
	float SpareLineKnockbackDistance = 300.f;

	// ---- Strongbox and interaction (2.3, 6.4) ----

	/** Max distance between the player and what they interact with. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "50.0", Units = "cm"))
	float InteractRange = 220.f;

	/** Hold Interact this long to lift a strongbox. Dropping is instant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (ClampMin = "0.0", Units = "s"))
	float PickupHoldSeconds = 0.5f;

	// ---- Quota (6.4) ----

	/** Quota = players + this... */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quota")
	int32 QuotaPlayersPlus = 1;

	/** ...but never more than this. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quota", meta = (ClampMin = "1"))
	int32 MaxQuota = 6;

	// ---- Flood Drill debug start (8.3) ----

	/** Tide clock the drill starts at (4:30). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flood Drill", meta = (ClampMin = "0.0", Units = "s"))
	float FloodDrillStartSeconds = 270.f;

	/** Drill quota = one strongbox per this many players, at least one. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flood Drill", meta = (ClampMin = "1"))
	int32 FloodDrillPlayersPerStrongbox = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flood Drill", meta = (ClampMin = "0"))
	int32 FloodDrillAttendants = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flood Drill", meta = (ClampMin = "0"))
	int32 FloodDrillScuttlers = 4;

	/** The drill's enemies are placed on the NavMesh between these distances from the drill start. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flood Drill", meta = (ClampMin = "0.0", Units = "cm"))
	float FloodDrillEnemyMinDistance = 1200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flood Drill", meta = (ClampMin = "0.0", Units = "cm"))
	float FloodDrillEnemyMaxDistance = 2500.f;
};
