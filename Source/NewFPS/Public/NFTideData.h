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
 * The class defaults are placeholders (4 minute cycle) so the prototype runs without any asset.
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

	// ---- Cycle ----

	/** Length of one full pass through Phases. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cycle", meta = (ClampMin = "1.0", Units = "s"))
	float CycleDurationSeconds = 240.f;

	/** Start over after the last phase. Off = the tide stops at the end of the last phase. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cycle")
	bool bLoop = true;

	/** The tide table, in order. Each phase should start at the level the previous one ended at. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cycle")
	TArray<FNFTidePhaseEntry> Phases;

	// ---- Height ----

	/** World Z (cm) of the water surface at level 0. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Height", meta = (Units = "cm"))
	float LowWaterZ = 0.f;

	/** World Z (cm) of the water surface at level 1. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Height", meta = (Units = "cm"))
	float HighWaterZ = 700.f;

	/**
	 * Shape of the level change inside one phase. X = 0..1 through the phase, Y = 0..1 from StartLevel to EndLevel.
	 * Empty = smooth ease in/out.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Height")
	TObjectPtr<UCurveFloat> HeightEaseCurve;

	// ---- Wetness thresholds (water depth above the feet) ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wetness", meta = (ClampMin = "0.0", Units = "cm"))
	float KneeDepth = 45.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wetness", meta = (ClampMin = "0.0", Units = "cm"))
	float WaistDepth = 105.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wetness", meta = (ClampMin = "0.0", Units = "cm"))
	float OverheadDepth = 175.f;

	/** The water must drop this far below a threshold before the state steps back down (stops flicker at the waterline). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Wetness", meta = (ClampMin = "0.0", Units = "cm"))
	float WetnessHysteresis = 4.f;

	// ---- Effects on characters ----

	/** Speed multiplier at exactly KneeDepth. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float KneeSpeedMultiplier = 0.75f;

	/** Speed multiplier at WaistDepth and deeper. Between knee and waist the multiplier is interpolated. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float WaistSpeedMultiplier = 0.5f;

	/** The slowdown ramps in over this many cm below KneeDepth instead of switching on at once. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "1.0", Units = "cm"))
	float WadeBlendDepth = 15.f;

	/** Weapons cannot be fired from this wetness upwards. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	ENFWetness WeaponsDisabledFrom = ENFWetness::Waist;

	/** Seconds with the head under water before drowning damage starts. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "0.0", Units = "s"))
	float DrownGraceSeconds = 6.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "0.1", Units = "s"))
	float DrownDamageInterval = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects", meta = (ClampMin = "0.0"))
	float DrownDamagePerInterval = 15.f;

	// ---- Warnings ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Warnings")
	TArray<FNFTideWarning> Warnings;

private:
	float GetTotalWeight() const;
	float EaseLevel(float Alpha) const;
	ENFWetness ClassifyDepth(float DepthCm) const;
};
