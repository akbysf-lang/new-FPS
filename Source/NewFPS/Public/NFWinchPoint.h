#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NFWinchPoint.generated.h"

class ANFStrongbox;
class UBoxComponent;
class USceneComponent;
class UStaticMeshComponent;

/**
 * The cargo net on the winch platform (prototype-spec.md 6.4): a strongbox dropped inside NetVolume is delivered
 * and counts towards the quota on ANFGameState. Place one per level, on the winch platform.
 *
 * Prototype simplification: a delivered strongbox counts at once and cannot be taken out again (the spec counts
 * at extraction). The winch line itself (hook on from 10:30) is not implemented.
 */
UCLASS()
class NEWFPS_API ANFWinchPoint : public AActor
{
	GENERATED_BODY()

public:
	ANFWinchPoint();

	/** Server only. True when the strongbox lies inside the net and was counted. */
	bool AuthTryDeliver(ANFStrongbox* Strongbox);

	UFUNCTION(BlueprintPure, Category = "Winch")
	bool IsInsideNet(const FVector& WorldLocation) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** The net: 400 x 400 cm by default. Scale or resize the box in the level. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> NetVolume;

	/** Flat placeholder so the net is visible in a grey box. Clear or replace the mesh when art exists. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> NetMarker;
};
