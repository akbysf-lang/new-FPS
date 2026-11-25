#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NFCharacterMovementComponent.generated.h"

/**
 * Character movement with two extra predicted states: sprint and aim-walk.
 * They travel inside the saved-move compressed flags (not RPCs), so client prediction and
 * server simulation always agree on the max speed.
 */
UCLASS()
class NEWFPS_API UNFCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual float GetMaxSpeed() const override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	/** Actually sprinting right now (wants to, moving forward, not crouched/aiming). */
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsSprinting() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking", meta = (ClampMin = "0.0"))
	float SprintSpeed = 750.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AimWalkSpeedMultiplier = 0.6f;

	/** Set by the owning client every frame; replicated to the server through compressed flags. */
	uint8 bWantsToSprint : 1;
	uint8 bWantsToAimWalk : 1;

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
};

class FNFSavedMove : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

	FNFSavedMove() : bSavedWantsToSprint(false), bSavedWantsToAimWalk(false) {}

	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
	virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
	virtual void PrepMoveFor(ACharacter* Character) override;

	uint8 bSavedWantsToSprint : 1;
	uint8 bSavedWantsToAimWalk : 1;
};

class FNFNetworkPredictionData_Client : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	explicit FNFNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};
