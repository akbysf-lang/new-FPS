#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NFLocalWaterLevel.h"
#include "NFSluiceZone.generated.h"

class UBoxComponent;

/**
 * Minimal sluice gate region (prototype): while the gate is closed, the water inside the box (a full-height
 * column; the box's Z extent is ignored) cannot rise above the level it had when the gate was closed.
 * After HoldSeconds the gate gives way and the zone is back on the global tide at once.
 *
 * Only the gate state is replicated; the height is again derived locally. The visual water plane of the tide
 * manager is global and does NOT show the dry zone yet (see the tide ADR).
 */
UCLASS()
class NEWFPS_API ANFSluiceZone : public AActor, public INFLocalWaterLevel
{
	GENERATED_BODY()

public:
	ANFSluiceZone();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// INFLocalWaterLevel
	virtual bool GetLocalWaterCap(const FVector& WorldLocation, float& OutMaxWaterZ) const override;

	/** Server only. Holds the water at its current height for HoldSeconds. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Sluice")
	void CloseGate();

	/** Server only. The zone follows the global tide again. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Sluice")
	void OpenGate();

	UFUNCTION(BlueprintPure, Category = "Sluice")
	bool IsGateClosed() const { return bGateClosed; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** The neighbourhood this gate keeps dry. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> ZoneBox;

	/** Seconds a closed gate holds before it gives way. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sluice", meta = (ClampMin = "0.1", Units = "s"))
	float HoldSeconds = 30.f;

private:
	UPROPERTY(Replicated)
	bool bGateClosed = false;

	/** Water Z captured when the gate closed. */
	UPROPERTY(Replicated)
	float HeldWaterZ = 0.f;

	FTimerHandle HoldTimerHandle;
};
