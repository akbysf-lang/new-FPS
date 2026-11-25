#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NFLocalWaterLevel.generated.h"

UINTERFACE(MinimalAPI)
class UNFLocalWaterLevel : public UInterface
{
	GENERATED_BODY()
};

/**
 * Extension point for regional water: anything that can hold the tide back in part of the level
 * (a closed sluice gate, a pumped-out cellar). Implementers register with UNFTideSubsystem.
 *
 * Contract: the answer must be computable identically on the server and on every client (replicate the
 * state that changes it, e.g. "gate closed", not the height), because movement prediction reads it.
 */
class NEWFPS_API INFLocalWaterLevel
{
	GENERATED_BODY()

public:
	/**
	 * @return true when WorldLocation is inside this modifier's region and the water there cannot rise
	 *         above OutMaxWaterZ right now. The local water is min(global tide, every cap that applies).
	 */
	virtual bool GetLocalWaterCap(const FVector& WorldLocation, float& OutMaxWaterZ) const = 0;
};
