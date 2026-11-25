#include "NFCharacterMovementComponent.h"
#include "NFTideData.h"
#include "NFTideManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Character.h"

namespace NFMoveConst
{
	/** Depth reported when the level has no tide. */
	const float NoWaterDepth = -100000.f;
}

UNFCharacterMovementComponent::UNFCharacterMovementComponent()
{
	bWantsToSprint = false;
	bWantsToAimWalk = false;
	bCarrying = false;
	bDowned = false;
}

float UNFCharacterMovementComponent::GetMaxSpeed() const
{
	const float BaseSpeed = Super::GetMaxSpeed();

	if (MovementMode != MOVE_Walking && MovementMode != MOVE_NavWalking && MovementMode != MOVE_Falling)
	{
		return BaseSpeed;
	}

	// Evaluated the same way by client prediction, client replay and the server.
	const float WaterMultiplier = GetWaterSpeedMultiplier();

	if (bDowned)
	{
		return DownedCrawlSpeed * WaterMultiplier;
	}
	if (IsCrouching())
	{
		return (bCarrying ? CarryCrouchSpeed : BaseSpeed) * WaterMultiplier;
	}
	if (IsSprinting())
	{
		return SprintSpeed * WaterMultiplier;
	}

	const float WalkSpeed = bCarrying ? CarryWalkSpeed : BaseSpeed;
	if (bWantsToAimWalk)
	{
		return WalkSpeed * AimWalkSpeedMultiplier * WaterMultiplier;
	}
	return WalkSpeed * WaterMultiplier;
}

float UNFCharacterMovementComponent::GetWaterDepth() const
{
	if (!CharacterOwner || !UpdatedComponent)
	{
		return NFMoveConst::NoWaterDepth;
	}

	const ANFTideManager* TideManager = ANFTideManager::Get(GetWorld());
	if (!TideManager)
	{
		return NFMoveConst::NoWaterDepth;
	}

	// UpdatedComponent, not the actor: during a client replay this is the replayed position.
	const FVector Location = UpdatedComponent->GetComponentLocation();
	const float FeetZ = static_cast<float>(Location.Z) - CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	return TideManager->GetWaterZAt(Location) - FeetZ;
}

float UNFCharacterMovementComponent::GetWaterSpeedMultiplier() const
{
	if (!bWaterSlowsMovement)
	{
		return 1.f;
	}

	const ANFTideManager* TideManager = ANFTideManager::Get(GetWorld());
	if (!TideManager)
	{
		return 1.f;
	}
	return TideManager->GetTideData()->GetSpeedMultiplierForDepth(GetWaterDepth());
}

bool UNFCharacterMovementComponent::IsSprinting() const
{
	if (!bWantsToSprint || bWantsToAimWalk || bCarrying || bDowned || IsCrouching() || !UpdatedComponent)
	{
		return false;
	}

	// No sprint from waist depth (spec 3.3). This one is a step, unlike the speed multiplier: at the threshold
	// the speed drops from sprint to walk on both sides within a few centimetres of each other.
	if (bWaterSlowsMovement)
	{
		if (const ANFTideManager* TideManager = ANFTideManager::Get(GetWorld()))
		{
			if (GetWaterDepth() >= TideManager->GetTideData()->WaistDepth)
			{
				return false;
			}
		}
	}

	// Forward only. Acceleration is part of the replicated move, so both sides evaluate this identically.
	const FVector AccelerationDirection = Acceleration.GetSafeNormal2D();
	return !AccelerationDirection.IsNearlyZero()
		&& FVector::DotProduct(AccelerationDirection, UpdatedComponent->GetForwardVector()) > 0.5;
}

bool UNFCharacterMovementComponent::CanAttemptJump() const
{
	if (bDowned || !Super::CanAttemptJump())
	{
		return false;
	}

	if (bWaterSlowsMovement)
	{
		if (const ANFTideManager* TideManager = ANFTideManager::Get(GetWorld()))
		{
			return GetWaterDepth() < TideManager->GetTideData()->WaistDepth;
		}
	}
	return true;
}

void UNFCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	// Runs for predicted, replayed and server moves alike, so the jump height always matches on both sides.
	float JumpMultiplier = 1.f;
	if (bWaterSlowsMovement)
	{
		if (const ANFTideManager* TideManager = ANFTideManager::Get(GetWorld()))
		{
			JumpMultiplier = TideManager->GetTideData()->GetJumpMultiplierForDepth(GetWaterDepth());
		}
	}
	JumpZVelocity = (bCarrying ? CarryJumpZVelocity : BaseJumpZVelocity) * JumpMultiplier;
}

void UNFCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bWantsToAimWalk = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
}

FNetworkPredictionData_Client* UNFCharacterMovementComponent::GetPredictionData_Client() const
{
	if (ClientPredictionData == nullptr)
	{
		UNFCharacterMovementComponent* MutableThis = const_cast<UNFCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNFNetworkPredictionData_Client(*this);
	}
	return ClientPredictionData;
}

// ---- FNFSavedMove ----

void FNFSavedMove::Clear()
{
	Super::Clear();
	bSavedWantsToSprint = false;
	bSavedWantsToAimWalk = false;
}

uint8 FNFSavedMove::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (bSavedWantsToSprint)
	{
		Result |= FLAG_Custom_0;
	}
	if (bSavedWantsToAimWalk)
	{
		Result |= FLAG_Custom_1;
	}
	return Result;
}

bool FNFSavedMove::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FNFSavedMove* Other = static_cast<const FNFSavedMove*>(NewMove.Get());
	if (bSavedWantsToSprint != Other->bSavedWantsToSprint || bSavedWantsToAimWalk != Other->bSavedWantsToAimWalk)
	{
		return false;
	}
	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FNFSavedMove::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	if (const UNFCharacterMovementComponent* Movement = Character ? Cast<UNFCharacterMovementComponent>(Character->GetCharacterMovement()) : nullptr)
	{
		bSavedWantsToSprint = Movement->bWantsToSprint;
		bSavedWantsToAimWalk = Movement->bWantsToAimWalk;
	}
}

void FNFSavedMove::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	if (UNFCharacterMovementComponent* Movement = Character ? Cast<UNFCharacterMovementComponent>(Character->GetCharacterMovement()) : nullptr)
	{
		Movement->bWantsToSprint = bSavedWantsToSprint;
		Movement->bWantsToAimWalk = bSavedWantsToAimWalk;
	}
}

// ---- FNFNetworkPredictionData_Client ----

FNFNetworkPredictionData_Client::FNFNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FNFNetworkPredictionData_Client::AllocateNewMove()
{
	return FSavedMovePtr(new FNFSavedMove());
}
