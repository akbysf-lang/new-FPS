#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NFMovementData.generated.h"

/**
 * Designer-tunable movement numbers. Applied to the character movement component at spawn.
 * Create in the editor: Content Browser -> Miscellaneous -> Data Asset -> NFMovementData.
 * The class defaults are used when the character has no asset assigned.
 */
UCLASS(BlueprintType)
class NEWFPS_API UNFMovementData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(FPrimaryAssetType(TEXT("NFMovement")), GetFName());
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0"))
	float WalkSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0"))
	float SprintSpeed = 750.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0"))
	float CrouchSpeed = 260.f;

	/** Walk speed multiplier while aiming down sights. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AimWalkSpeedMultiplier = 0.6f;

	/** cm/s^2. 500 cm/s at 5000 = full speed in 0.1 s. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0"))
	float MaxAcceleration = 5000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0"))
	float BrakingDeceleration = 4000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0"))
	float GroundFriction = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Air", meta = (ClampMin = "0.0"))
	float JumpZVelocity = 520.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Air", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AirControl = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Air", meta = (ClampMin = "0.0"))
	float GravityScale = 1.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crouch", meta = (ClampMin = "20.0"))
	float CrouchedHalfHeight = 60.f;
};
