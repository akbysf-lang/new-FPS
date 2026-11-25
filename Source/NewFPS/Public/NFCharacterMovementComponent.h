#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NFCharacterMovementComponent.generated.h"

/**
 * Character movement with two extra predicted states: sprint and aim-walk.
 * They travel inside the saved-move compressed flags (not RPCs), so client prediction and
 * server simulation always agree on the max speed.
 *
 * Wading in the tide also scales the max speed. That needs no flag at all: the multiplier is a pure function
 * of the character's position (part of every move) and the deterministic water height (see ANFTideManager).
 *
 * Carrying a strongbox and being downed change the speeds too. Those two are server-authoritative, replicated
 * states (they start with a server action), so they are plain members set on every machine by the character.
 * The owning client learns about them one latency late; expect a single small correction when they change.
 */
UCLASS()
class NEWFPS_API UNFCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UNFCharacterMovementComponent();

	virtual float GetMaxSpeed() const override;
	virtual bool CanAttemptJump() const override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	/** Actually sprinting right now (wants to, moving forward, not crouched/aiming/carrying, not waist deep). */
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsSprinting() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking", meta = (ClampMin = "0.0"))
	float SprintSpeed = 650.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AimWalkSpeedMultiplier = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking", meta = (ClampMin = "0.0"))
	float CarryWalkSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking", meta = (ClampMin = "0.0"))
	float CarryCrouchSpeed = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking", meta = (ClampMin = "0.0"))
	float DownedCrawlSpeed = 100.f;

	/** Dry, unloaded jump velocity. JumpZVelocity itself is rewritten every move from this, the load and the water. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Jumping / Falling", meta = (ClampMin = "0.0"))
	float BaseJumpZVelocity = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Jumping / Falling", meta = (ClampMin = "0.0"))
	float CarryJumpZVelocity = 360.f;

	/** Water depth above the feet right now (cm); very negative when the level has no tide. */
	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetWaterDepth() const;

	/** Speed multiplier from the water depth at the feet right now (1 = dry or no tide in the level). */
	UFUNCTION(BlueprintPure, Category = "Movement")
	float GetWaterSpeedMultiplier() const;

	/** Off for things that are not slowed by water. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking")
	bool bWaterSlowsMovement = true;

	/** Set by the owning client every frame; replicated to the server through compressed flags. */
	uint8 bWantsToSprint : 1;
	uint8 bWantsToAimWalk : 1;

	/** Set by ANFCharacter on every machine from replicated state (see the class comment). */
	uint8 bCarrying : 1;
	uint8 bDowned : 1;

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
