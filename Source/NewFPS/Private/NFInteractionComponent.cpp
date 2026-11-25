#include "NFInteractionComponent.h"
#include "NewFPS.h"
#include "NFCharacter.h"
#include "NFRulesData.h"
#include "NFStrongbox.h"
#include "NFTideData.h"
#include "NFWetnessComponent.h"
#include "NFWinchPoint.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

namespace NFInteraction
{
	/** The range test is horizontal; this caps the height difference (cm). */
	const float MaxHeightDifference = 220.f;
	/** A hold survives the target drifting this much beyond the start range. */
	const float KeepRangeScale = 1.3f;
	/** The strongbox is put down this far in front of the carrier (cm). */
	const float DropForwardDistance = 70.f;
}

UNFInteractionComponent::UNFInteractionComponent()
{
	// Ticks on the server only, and only while a hold is running.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}

void UNFInteractionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UNFInteractionComponent, CarriedBox);
	DOREPLIFETIME_CONDITION(UNFInteractionComponent, ActiveInteraction, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UNFInteractionComponent, ActiveInteractionDuration, COND_OwnerOnly);
}

void UNFInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// A carrier that is destroyed mid-mission (respawn, disconnect) leaves the strongbox where it stood.
	if (EndPlayReason == EEndPlayReason::Destroyed && GetOwner() && GetOwner()->HasAuthority() && CarriedBox)
	{
		AuthDropCarried();
	}

	Super::EndPlay(EndPlayReason);
}

ANFCharacter* UNFInteractionComponent::GetNFOwner() const
{
	return Cast<ANFCharacter>(GetOwner());
}

float UNFInteractionComponent::GetWorldTime() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.f;
}

bool UNFInteractionComponent::IsWithinRange(const AActor* Other, float Range) const
{
	const AActor* Owner = GetOwner();
	if (!Owner || !Other)
	{
		return false;
	}
	const FVector Delta = Other->GetActorLocation() - Owner->GetActorLocation();
	return Delta.SizeSquared2D() <= FMath::Square(Range) && FMath::Abs(Delta.Z) <= NFInteraction::MaxHeightDifference;
}

float UNFInteractionComponent::GetInteractionProgress() const
{
	if (ActiveInteraction == ENFInteraction::None)
	{
		return 0.f;
	}
	if (ActiveInteractionDuration <= KINDA_SMALL_NUMBER)
	{
		return 1.f;
	}
	return FMath::Clamp((GetWorldTime() - InteractionStartTime) / ActiveInteractionDuration, 0.f, 1.f);
}

// ---------------------------------------------------------------------------------------------
// Input (owning client)
// ---------------------------------------------------------------------------------------------

void UNFInteractionComponent::BeginInteract()
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (CarriedBox)
	{
		RequestDrop();
		return;
	}

	if (Owner->HasAuthority())
	{
		AuthBeginInteraction();
	}
	else
	{
		ServerSetInteracting(true);
	}
}

void UNFInteractionComponent::EndInteract()
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	if (Owner->HasAuthority())
	{
		AuthCancelInteraction();
	}
	else
	{
		ServerSetInteracting(false);
	}
}

void UNFInteractionComponent::RequestDrop()
{
	const AActor* Owner = GetOwner();
	if (!Owner || !CarriedBox)
	{
		return;
	}

	if (Owner->HasAuthority())
	{
		AuthDropCarried();
	}
	else
	{
		ServerDropCarried();
	}
}

void UNFInteractionComponent::ServerSetInteracting_Implementation(bool bInteracting)
{
	if (bInteracting)
	{
		AuthBeginInteraction();
	}
	else
	{
		AuthCancelInteraction();
	}
}

void UNFInteractionComponent::ServerDropCarried_Implementation()
{
	AuthDropCarried();
}

// ---------------------------------------------------------------------------------------------
// Hold interactions (server)
// ---------------------------------------------------------------------------------------------

void UNFInteractionComponent::AuthBeginInteraction()
{
	ANFCharacter* Owner = GetNFOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World || !Owner->HasAuthority() || Owner->GetLifeState() != ENFLifeState::Alive || CarriedBox)
	{
		return;
	}

	const UNFRulesData* Rules = Owner->GetRules();
	const FVector OwnerLocation = Owner->GetActorLocation();

	// A downed teammate comes before the haul.
	ANFCharacter* BestTeammate = nullptr;
	double BestTeammateDistance = TNumericLimits<double>::Max();
	for (TActorIterator<ANFCharacter> It(World); It; ++It)
	{
		ANFCharacter* Other = *It;
		if (Other == Owner || Other->GetLifeState() != ENFLifeState::Downed || !IsWithinRange(Other, Rules->InteractRange))
		{
			continue;
		}
		const double Distance = FVector::DistSquared(OwnerLocation, Other->GetActorLocation());
		if (Distance < BestTeammateDistance)
		{
			BestTeammateDistance = Distance;
			BestTeammate = Other;
		}
	}

	if (BestTeammate)
	{
		AuthSetInteraction(ENFInteraction::Revive, BestTeammate, Rules->ReviveHoldSeconds);
		return;
	}

	// A strongbox cannot be lifted from chest depth (spec 3.3); one already in hand stays there.
	if (const UNFWetnessComponent* Wetness = Owner->GetWetnessComponent())
	{
		if (Wetness->GetWaterDepth() >= Wetness->GetTideData()->ChestDepth)
		{
			return;
		}
	}

	ANFStrongbox* BestBox = nullptr;
	double BestBoxDistance = TNumericLimits<double>::Max();
	for (TActorIterator<ANFStrongbox> It(World); It; ++It)
	{
		ANFStrongbox* Box = *It;
		if (!Box->CanBePickedUp() || !IsWithinRange(Box, Rules->InteractRange))
		{
			continue;
		}
		const double Distance = FVector::DistSquared(OwnerLocation, Box->GetActorLocation());
		if (Distance < BestBoxDistance)
		{
			BestBoxDistance = Distance;
			BestBox = Box;
		}
	}

	if (BestBox)
	{
		AuthSetInteraction(ENFInteraction::PickUp, BestBox, Rules->PickupHoldSeconds);
	}
}

void UNFInteractionComponent::AuthSetInteraction(ENFInteraction NewInteraction, AActor* NewTarget, float Duration)
{
	ActiveInteraction = NewInteraction;
	ActiveInteractionDuration = FMath::Max(Duration, 0.f);
	InteractionTarget = NewTarget;
	InteractionStartTime = GetWorldTime();
	SetComponentTickEnabled(NewInteraction != ENFInteraction::None);
}

void UNFInteractionComponent::AuthCancelInteraction()
{
	if (GetOwner() && GetOwner()->HasAuthority() && ActiveInteraction != ENFInteraction::None)
	{
		AuthSetInteraction(ENFInteraction::None, nullptr, 0.f);
	}
}

bool UNFInteractionComponent::AuthIsInteractionStillValid() const
{
	const ANFCharacter* Owner = GetNFOwner();
	if (!Owner || Owner->GetLifeState() != ENFLifeState::Alive || !InteractionTarget)
	{
		return false;
	}

	const float KeepRange = Owner->GetRules()->InteractRange * NFInteraction::KeepRangeScale;
	if (!IsWithinRange(InteractionTarget, KeepRange))
	{
		return false;
	}

	switch (ActiveInteraction)
	{
	case ENFInteraction::PickUp:
	{
		const ANFStrongbox* Box = Cast<ANFStrongbox>(InteractionTarget);
		return Box && Box->CanBePickedUp() && !CarriedBox;
	}
	case ENFInteraction::Revive:
	{
		const ANFCharacter* Teammate = Cast<ANFCharacter>(InteractionTarget);
		return Teammate && Teammate->GetLifeState() == ENFLifeState::Downed;
	}
	default:
		return false;
	}
}

void UNFInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner() || !GetOwner()->HasAuthority() || ActiveInteraction == ENFInteraction::None)
	{
		SetComponentTickEnabled(false);
		return;
	}

	if (!AuthIsInteractionStillValid())
	{
		AuthCancelInteraction();
		return;
	}

	if (GetWorldTime() - InteractionStartTime >= ActiveInteractionDuration)
	{
		AuthCompleteInteraction();
	}
}

void UNFInteractionComponent::AuthCompleteInteraction()
{
	ANFCharacter* Owner = GetNFOwner();
	const ENFInteraction Finished = ActiveInteraction;
	AActor* Target = InteractionTarget;
	AuthSetInteraction(ENFInteraction::None, nullptr, 0.f);

	if (!Owner || !Target)
	{
		return;
	}

	if (Finished == ENFInteraction::PickUp)
	{
		if (ANFStrongbox* Box = Cast<ANFStrongbox>(Target))
		{
			Box->AuthSetCarrier(Owner);
			CarriedBox = Box;
			ApplyCarryState();
		}
	}
	else if (Finished == ENFInteraction::Revive)
	{
		if (ANFCharacter* Teammate = Cast<ANFCharacter>(Target))
		{
			const UNFRulesData* Rules = Owner->GetRules();
			Teammate->AuthReviveFromDowned(Rules->ReviveHealth, Rules->ReviveInvulnerableSeconds);
		}
	}
}

// ---------------------------------------------------------------------------------------------
// Carrying
// ---------------------------------------------------------------------------------------------

void UNFInteractionComponent::AuthDropCarried()
{
	AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World || !Owner->HasAuthority() || !CarriedBox)
	{
		return;
	}

	ANFStrongbox* Box = CarriedBox;
	CarriedBox = nullptr;

	// Put it down just in front of the carrier, on whatever floor is there. Never thrown.
	const FVector OwnerLocation = Owner->GetActorLocation();
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(NFStrongboxDrop), /*bTraceComplex*/ false, Owner);
	QueryParams.AddIgnoredActor(Box);
	const FCollisionObjectQueryParams StaticWorld(ECC_WorldStatic);

	FVector DropColumn = OwnerLocation + Owner->GetActorForwardVector() * NFInteraction::DropForwardDistance;
	if (World->LineTraceTestByObjectType(OwnerLocation, DropColumn, StaticWorld, QueryParams))
	{
		// A wall is in the way: drop it where the carrier stands.
		DropColumn = OwnerLocation;
	}

	float FeetOffset = 0.f;
	if (const ANFCharacter* Character = GetNFOwner())
	{
		FeetOffset = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}

	FVector FloorPoint = FVector(DropColumn.X, DropColumn.Y, OwnerLocation.Z - FeetOffset);
	FHitResult FloorHit;
	if (World->LineTraceSingleByObjectType(FloorHit, DropColumn, DropColumn - FVector(0.f, 0.f, 600.f), StaticWorld, QueryParams))
	{
		FloorPoint = FloorHit.ImpactPoint;
	}

	Box->AuthDropAt(FloorPoint + FVector(0.f, 0.f, Box->GetRestHalfHeight()), FRotator(0.f, Owner->GetActorRotation().Yaw, 0.f));
	ApplyCarryState();

	for (TActorIterator<ANFWinchPoint> It(World); It; ++It)
	{
		if (It->AuthTryDeliver(Box))
		{
			break;
		}
	}
}

void UNFInteractionComponent::OnRep_CarriedBox()
{
	ApplyCarryState();
}

void UNFInteractionComponent::OnRep_ActiveInteraction()
{
	// The owning client measures the hold from the moment it hears about it.
	InteractionStartTime = GetWorldTime();
}

void UNFInteractionComponent::ApplyCarryState()
{
	if (ANFCharacter* Owner = GetNFOwner())
	{
		Owner->HandleCarryStateChanged(CarriedBox != nullptr);
	}
}
