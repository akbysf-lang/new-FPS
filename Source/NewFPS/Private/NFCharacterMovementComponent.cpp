#include "NFCharacterMovementComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Character.h"

float UNFCharacterMovementComponent::GetMaxSpeed() const
{
	const float BaseSpeed = Super::GetMaxSpeed();

	if (MovementMode != MOVE_Walking && MovementMode != MOVE_NavWalking && MovementMode != MOVE_Falling)
	{
		return BaseSpeed;
	}
	if (IsCrouching())
	{
		return BaseSpeed;
	}
	if (IsSprinting())
	{
		return SprintSpeed;
	}
	if (bWantsToAimWalk)
	{
		return BaseSpeed * AimWalkSpeedMultiplier;
	}
	return BaseSpeed;
}

bool UNFCharacterMovementComponent::IsSprinting() const
{
	if (!bWantsToSprint || bWantsToAimWalk || IsCrouching() || !UpdatedComponent)
	{
		return false;
	}

	// Forward only. Acceleration is part of the replicated move, so both sides evaluate this identically.
	const FVector AccelerationDirection = Acceleration.GetSafeNormal2D();
	return !AccelerationDirection.IsNearlyZero()
		&& FVector::DotProduct(AccelerationDirection, UpdatedComponent->GetForwardVector()) > 0.5;
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
