#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NFTypes.h"
#include "NFTideData.generated.h"

class UCurveFloat;

/** One row of the tide table. */
USTRUCT(BlueprintType)
struct FNFTidePhaseEntry
{
	GENERATED_BODY()

	/** Label used by the HUD, bells and announcements. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tide")
	ENFTidePhase Phase = ENFTidePhase::LowSlack;

	/** Share of the cycle this phase takes. Seconds = CycleDurationSeconds * Weight / sum of all weights. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tide", meta = (ClampMin = "0.0"))
	float DurationWeight = 1.f;

	/** Water level when the phase starts: 0 = LowWaterZ, 1 = HighWaterZ. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tide", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StartLevel = 0.f;

	/** Water level when the phase ends: 0 = LowWaterZ, 1 = HighWaterZ. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tide", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EndLevel = 0.f;
};

/** A bell/announcement cue fired some seconds before a phase of the given kind begins. */
USTRUCT(BlueprintType)
struct FNFTideWarning
{
	GENERATED_BODY()

	/** Passed to ANFTideManager::OnTideWarning so audio/UI can pick the right cue. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tide")
	FName WarningId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tide")
	ENFTidePhase UpcomingPhase = ENFTidePhase::Flood;

	/** Must be shorter than the phase that precedes UpcomingPhase, otherwise the warning never fires. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tide", meta = (ClampMin = "0.0"))
	float SecondsBefore = 10.f;
};

/** The tide at one moment. Pure function of the tide data and the elapsed time. */
USTRUCT(BlueprintType)
struct FNFTideSample
{
	GENERATED_BODY()

	/** False when the tide table is empty; the other fields then describe still water at LowWaterZ. */
	UPROPERTY(BlueprintReadOnly, Category = "Tide")
	bool bValid = false;

	UPROPERTY(BlueprintReadOnly, Category = "Tide")
	int32 CycleIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tide")
	int32 PhaseIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tide")
	ENFTidePhase Phase = ENFTidePhase::LowSlack;

	/** Label of the phase that follows this one. */
	UPROPERTY(BlueprintReadOnly, Category = "Tide")
	ENFTidePhase NextPhase = ENFTidePhase::LowSlack;

	UPROPERTY(BlueprintReadOnly, Category = "Tide")
	float PhaseDuration = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Tide")
	float PhaseElapsed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Tide")
	float PhaseRemaining = 0.f;

	/**
	 * Seconds until the phase LABEL changes (e.g. until the tide turns): the rest of this entry plus every
	 * following entry with the same label. This is what the HUD clock and the warnings count down.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Tide")
	float LabelRemaining = 0.f;

	/** The label that follows the current run of same-label entries (== Phase when nothing follows). */
	UPROPERTY(BlueprintReadOnly, Category = "Tide")
	ENFTidePhase NextLabel = ENFTidePhase::LowSlack;

	/** 0 = low water, 1 = high water. */
	UPROPERTY(BlueprintReadOnly, Category = "Tide")
	float Level = 0.f;

	/** World Z of the water surface (before any per-level offset of the tide manager). */
	UPROPERTY(BlueprintReadOnly, Category = "Tide")
	float WaterZ = 0.f;
};

/**
 * Designer-tunable tide table and wetness rules for one mission.
 * Create in the editor: Content Browser -> Miscellaneous -> Data Asset -> NFTideData, assign it to the level's ANFTideManager.
 * The class defaults are the prototype spec's 12 minute Promenade table (prototype-spec.md 3.2), so the
 * prototype runs without any asset.
 */
UCLASS(BlueprintType)
class NEWFPS_API UNFTideData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UNFTideData();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** The tide after ElapsedSeconds on the tide clock. Deterministic: same input, same output on every machine. */
	FNFTideSample Evaluate(double ElapsedSeconds) const;

	/** Seconds the phase at PhaseIndex lasts. */
	UFUNCTION(BlueprintPure, Category = "Tide")
	float GetPhaseDuration(int32 PhaseIndex) const;

	/** Wetness for a water depth above the feet (cm). CurrentWetness adds hysteresis on the way down. */
	UFUNCTION(BlueprintPure, Category = "Tide")
	ENFWetness GetWetnessForDepth(float DepthCm, ENFWetness CurrentWetness) const;

	/**
	 * Movement speed multiplier for a water depth above the feet (cm). Continuous on purpose: client prediction
	 * and the server evaluate it a few centimetres apart, and a step function would cause corrections.
	 */
	UFUNCTION(BlueprintPure, Category = "Tide")
	float GetSpeedMultiplierForDepth(float DepthCm) const;

	/** Jump velocity multiplier for a water depth above the feet (cm). Jumping stops altogether at WaistDepth. */
	UFUNCTION(BlueprintPure, Category = "Tide")
	float GetJumpMultiplierForDepth(float DepthCm) const;

	/**
	 * Depth (cm) from which a firearm that is usable up to MaxUsableWetness stops working: the threshold of the
	 * next wetter state, never deeper than ChestDepth (the common lock for every firearm).
	 */
	UFUNCTION(BlueprintPure, Category = "Tide")
	float GetFirearmDepthLimit(ENFWetness MaxUsableWetness) const;

	// ---- Cycle ----

	/** Length of one full pass through Phases. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cycle", meta = (ClampMin = "1.0", Units = "s"))
	float CycleDurationSeconds = 720.f;

	/** Start over after the last phase. Off = the tide stops at the end of the last phase. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cycle")
	bool bLoop = false;

	/** The tide table, in order. Each phase should start at the level the previous one ended at. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cycle")
	TArray<FNFTidePhaseEntry> Phases;

	// ---- Height ----

	/** World Z (cm) of the water surface at level 0. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Height", meta = (Units = "cm"))
	float LowWaterZ = 0.f;

	/** World Z (cm) of the water surface at level 1. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Height", meta = (Units = "cm"))
	float HighWaterZ = 830.f;

	/**
	 * Shape of the level change inside one phase. X = 0..1 through the phase, Y = 0..1 from StartLevel to EndLevel.
	 * Empty = linear, or smooth ease in/out when bSmoothPhaseEase is set.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Height")
	TObjectPtr<UCurveFloat> HeightEaseCurve;

	/** Only without a HeightEaseCurve. Off = piecewise linear water (the spec's table); on = smoothstep per entry. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Height")
	bool bSmoothPhaseEase = false;

	// ---- Wetness thresholds (water depth above the feet) ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wetness", meta = (ClampMin = "0.0", Units = "cm"))
	float KneeDepth = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wetness", meta = (ClampMin = "0.0", Units = "cm"))
	float WaistDepth = 100.f;

	/**
	 * Not a wetness state: a depth inside Waist. From here no firearm works (only the Kedge), a strongbox cannot
	 * be picked up, and the HUD shows "TOO DEEP TO SHOOT".
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wetness", meta = (ClampMin = "0.0", Units = "cm"))
	float ChestDepth = 135.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wetness", meta = (ClampMin = "0.0", Units = "cm"))
	float OverheadDepth = 165.f;

	/** The water must drop this far below a threshold before the state steps back down (stops flicker at the waterline). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wetness", meta = (ClampMin = "0.0", Units = "cm"))
	float WetnessHysteresis = 8.f;

	/** A new state must hold this long before it is adopted (second guard against flicker at the waterline). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wetness", meta = (ClampMin = "0.0", Units = "s"))
	float WetnessChangeDelay = 0.2f;

	// ---- Effects on characters ----

	/** Speed multiplier at exactly KneeDepth. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float KneeSpeedMultiplier = 0.75f;

	/** Speed multiplier at exactly WaistDepth. Between knee and waist the multiplier is interpolated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float WaistSpeedMultiplier = 0.5f;

	/** Speed multiplier at ChestDepth and deeper. Between waist and chest the multiplier is interpolated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float ChestSpeedMultiplier = 0.4f;

	/** Jump velocity multiplier at KneeDepth (blended in like the speed). No jumping at all from WaistDepth. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float KneeJumpMultiplier = 0.8f;

	/** The slowdown ramps in over this many cm below KneeDepth instead of switching on at once. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "1.0", Units = "cm"))
	float WadeBlendDepth = 15.f;

	/**
	 * Firearm rule (spec 3.3, bound to depth, not to the wetness state):
	 *   depth < WaistDepth               every weapon
	 *   WaistDepth <= depth < ChestDepth  only weapons with MaxUsableWetness >= Waist (Purser), hip-fire only,
	 *                                     spread x WaistSpreadMultiplier, reload time x WaistReloadTimeMultiplier
	 *   depth >= ChestDepth               no firearm, Kedge only
	 *   depth >= OverheadDepth            nothing
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "1.0"))
	float WaistSpreadMultiplier = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "1.0"))
	float WaistReloadTimeMultiplier = 1.25f;

	/** Seconds with the head under water before drowning damage starts (spec: 8 s of breath, then 20 health/s). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "0.0", Units = "s"))
	float DrownGraceSeconds = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "0.1", Units = "s"))
	float DrownDamageInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "0.0"))
	float DrownDamagePerInterval = 10.f;

	// ---- Warnings ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warnings")
	TArray<FNFTideWarning> Warnings;

private:
	float GetTotalWeight() const;
	float EaseLevel(float Alpha) const;
	ENFWetness ClassifyDepth(float DepthCm) const;
};
