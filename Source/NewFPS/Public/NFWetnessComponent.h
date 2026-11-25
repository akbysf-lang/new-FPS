#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NFTypes.h"
#include "NFWetnessComponent.generated.h"

class UNFTideData;
class UNFWetnessComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNFOnWetnessChanged, UNFWetnessComponent*, WetnessComponent, ENFWetness, NewWetness, ENFWetness, OldWetness);

/**
 * How deep the owner stands in the tide: Dry / Knee / Waist / Overhead. Shared by players and AI.
 *
 * Not replicated. The water height is deterministic (ANFTideManager) and the owner's position is already
 * replicated, so every machine evaluates the same state on its own, on a low-frequency timer instead of a tick.
 */
UCLASS(ClassGroup = (NewFPS), meta = (BlueprintSpawnableComponent))
class NEWFPS_API UNFWetnessComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNFWetnessComponent();

	UFUNCTION(BlueprintPure, Category = "Wetness")
	ENFWetness GetWetness() const { return Wetness; }

	UFUNCTION(BlueprintPure, Category = "Wetness")
	bool IsAtLeast(ENFWetness Level) const { return Wetness >= Level; }

	/** Water depth above the feet at the last update (cm). Negative = the water is below the feet. */
	UFUNCTION(BlueprintPure, Category = "Wetness")
	float GetWaterDepth() const { return WaterDepth; }

	/** World Z of the owner's feet (capsule bottom for characters). */
	UFUNCTION(BlueprintPure, Category = "Wetness")
	float GetFeetZ() const;

	/** Thresholds and effect numbers in use: the level's tide data, or the UNFTideData class defaults. Never null. */
	UFUNCTION(BlueprintPure, Category = "Wetness")
	const UNFTideData* GetTideData() const;

	/** Re-evaluate now instead of waiting for the timer (e.g. right after a teleport). */
	UFUNCTION(BlueprintCallable, Category = "Wetness")
	void RefreshWetness();

	/** Fires locally on every machine when the state changes. */
	UPROPERTY(BlueprintAssignable, Category = "Wetness")
	FNFOnWetnessChanged OnWetnessChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Seconds between updates. The tide moves a few cm per second, so 10 Hz is plenty. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wetness", meta = (ClampMin = "0.02", Units = "s"))
	float UpdateInterval = 0.1f;

	/** Added to the feet height (cm), for owners whose origin/bounds do not describe where they stand. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wetness", meta = (Units = "cm"))
	float FeetZOffset = 0.f;

private:
	ENFWetness Wetness = ENFWetness::Dry;
	float WaterDepth = -100000.f;
	FTimerHandle UpdateTimerHandle;
};
