#include "NFWetnessComponent.h"
#include "NFTideData.h"
#include "NFTideManager.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"

namespace NFWetness
{
	/** Depth reported when the level has no tide. */
	const float NoWaterDepth = -100000.f;
}

UNFWetnessComponent::UNFWetnessComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNFWetnessComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		// Random first delay so a crowd of AI does not update in the same frame.
		const float Interval = FMath::Max(UpdateInterval, 0.02f);
		World->GetTimerManager().SetTimer(UpdateTimerHandle, this, &UNFWetnessComponent::RefreshWetness, Interval, true, FMath::FRandRange(0.f, Interval));
	}
}

void UNFWetnessComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

float UNFWetnessComponent::GetFeetZ() const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return 0.f;
	}

	float HalfHeight = 0.f;
	if (const ACharacter* Character = Cast<ACharacter>(Owner))
	{
		// Same formula as UNFCharacterMovementComponent::GetWaterSpeedMultiplier.
		HalfHeight = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}
	else
	{
		HalfHeight = Owner->GetSimpleCollisionHalfHeight();
	}
	return static_cast<float>(Owner->GetActorLocation().Z) - HalfHeight + FeetZOffset;
}

const UNFTideData* UNFWetnessComponent::GetTideData() const
{
	if (const ANFTideManager* TideManager = ANFTideManager::Get(GetWorld()))
	{
		return TideManager->GetTideData();
	}
	return GetDefault<UNFTideData>();
}

void UNFWetnessComponent::RefreshWetness()
{
	const AActor* Owner = GetOwner();
	const ANFTideManager* TideManager = ANFTideManager::Get(GetWorld());

	ENFWetness NewWetness = ENFWetness::Dry;
	float ChangeDelay = 0.f;
	if (Owner && TideManager)
	{
		const UNFTideData* TideData = TideManager->GetTideData();
		WaterDepth = TideManager->GetWaterZAt(Owner->GetActorLocation()) - GetFeetZ();
		NewWetness = TideData->GetWetnessForDepth(WaterDepth, Wetness);
		ChangeDelay = TideData->WetnessChangeDelay;
	}
	else
	{
		WaterDepth = NFWetness::NoWaterDepth;
	}

	if (NewWetness == Wetness)
	{
		PendingWetness = Wetness;
		return;
	}

	// The depth (GetWaterDepth) is always current; only the state change waits out the delay.
	const UWorld* World = GetWorld();
	const float Now = World ? World->GetTimeSeconds() : 0.f;
	if (NewWetness != PendingWetness)
	{
		PendingWetness = NewWetness;
		PendingSinceTime = Now;
	}

	if (ChangeDelay <= 0.f || Now - PendingSinceTime >= ChangeDelay)
	{
		const ENFWetness OldWetness = Wetness;
		Wetness = NewWetness;
		OnWetnessChanged.Broadcast(this, NewWetness, OldWetness);
	}
}
