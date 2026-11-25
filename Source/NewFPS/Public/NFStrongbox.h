#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NFStrongbox.generated.h"

class UStaticMeshComponent;

/**
 * The haul: a 60 x 40 x 40 cm box one player carries under the left arm (prototype-spec.md 2.3, 6.4).
 * Picking up and dropping run on the server through UNFInteractionComponent. While carried the box actor is
 * hidden and has no collision; the carrier shows a proxy mesh instead. It is dropped where the carrier stands,
 * never thrown, and stays where it lies when the tide covers it.
 */
UCLASS()
class NEWFPS_API ANFStrongbox : public AActor
{
	GENERATED_BODY()

public:
	ANFStrongbox();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Lying free: not carried and not yet on the winch net. */
	UFUNCTION(BlueprintPure, Category = "Strongbox")
	bool CanBePickedUp() const { return Carrier == nullptr && !bDelivered; }

	UFUNCTION(BlueprintPure, Category = "Strongbox")
	AActor* GetCarrier() const { return Carrier; }

	UFUNCTION(BlueprintPure, Category = "Strongbox")
	bool IsDelivered() const { return bDelivered; }

	/** Half height of the box, for resting it on a floor. */
	float GetRestHalfHeight() const { return BoxSize.Z * 0.5f; }

	// ---- Server only ----

	void AuthSetCarrier(AActor* NewCarrier);
	/** Put the box down at a world location (its centre) and make it visible again. */
	void AuthDropAt(const FVector& WorldLocation, const FRotator& WorldRotation);
	/** On the winch net: counted, and no longer picked up. */
	void AuthMarkDelivered();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_State();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BoxMesh;

	/** Size of the placeholder box (cm). Replace the mesh on BoxMesh in a Blueprint when the model exists. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Strongbox")
	FVector BoxSize = FVector(60.f, 40.f, 40.f);

private:
	void ApplyState();

	UPROPERTY(ReplicatedUsing = OnRep_State)
	TObjectPtr<AActor> Carrier;

	UPROPERTY(ReplicatedUsing = OnRep_State)
	bool bDelivered = false;
};
