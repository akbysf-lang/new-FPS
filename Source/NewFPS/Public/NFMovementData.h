#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NFMovementData.generated.h"

/**
 * Designer-tunable 3C numbers (prototype-spec.md section 2). Applied to the character at spawn.
 * Create in the editor: Content Browser -> Miscellaneous -> Data Asset -> NFMovementData.
 * The class defaults are the spec's starting values and are used when the character has no asset assigned.
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

	// ---- Body (cm) ----

	// The standing capsule (radius 34, half height 90) is NOT here: the engine restores the capsule from the
	// character class defaults when a crouch ends, so it has to be edited on the character itself
	// (BP_NFCharacter -> CapsuleComponent). The eye heights below are measured from the floor.

	/** Crouched capsule half height. 55 = the capsule top is level with a 110 cm low cover. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Body", meta = (ClampMin = "20.0", Units = "cm"))
	float CrouchedHalfHeight = 55.f;

	/** Eye height above the floor while standing. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Body", meta = (ClampMin = "10.0", Units = "cm"))
	float StandingEyeHeight = 164.f;

	/** Eye height above the floor while crouched. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Body", meta = (ClampMin = "10.0", Units = "cm"))
	float CrouchedEyeHeight = 104.f;

	// ---- Ground ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0"))
	float WalkSpeed = 500.f;

	/** Forward only (+-45 degrees); no firing while sprinting. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0"))
	float SprintSpeed = 650.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0"))
	float CrouchSpeed = 250.f;

	/** Walk speed multiplier while aiming down sights (0.6 x 500 = 300). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AimWalkSpeedMultiplier = 0.6f;

	/** cm/s^2. 500 cm/s at 5000 = full speed in 0.1 s. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0"))
	float MaxAcceleration = 5000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0"))
	float BrakingDeceleration = 4000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0"))
	float GroundFriction = 8.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0", Units = "cm"))
	float MaxStepHeight = 45.f;

	/** Steepest walkable slope in degrees (the steepest ramp of the map is 26.6). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ground", meta = (ClampMin = "0.0", ClampMax = "89.0"))
	float WalkableFloorAngle = 40.f;

	// ---- Air ----

	/** 450 at gravity 1.2 = an 86 cm jump: a 110 cm low cover cannot be jumped. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Air", meta = (ClampMin = "0.0"))
	float JumpZVelocity = 450.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Air", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AirControl = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Air", meta = (ClampMin = "0.0"))
	float GravityScale = 1.2f;

	// ---- Carrying a strongbox (spec 2.3). No sprint while carrying. ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Carry", meta = (ClampMin = "0.0"))
	float CarryWalkSpeed = 400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Carry", meta = (ClampMin = "0.0"))
	float CarryCrouchSpeed = 200.f;

	/** 360 = a 55 cm hop: steps and small obstacles only. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Carry", meta = (ClampMin = "0.0"))
	float CarryJumpZVelocity = 360.f;

	// ---- Downed (spec 6.2) ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Downed", meta = (ClampMin = "0.0"))
	float DownedCrawlSpeed = 100.f;
};
