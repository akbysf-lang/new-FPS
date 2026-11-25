#include "NFSluiceZone.h"
#include "NFTideManager.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

ANFSluiceZone::ANFSluiceZone()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	ZoneBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneBox"));
	ZoneBox->SetBoxExtent(FVector(1000.f, 1000.f, 500.f));
	ZoneBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ZoneBox->SetGenerateOverlapEvents(false);
	ZoneBox->SetCanEverAffectNavigation(false);
	RootComponent = ZoneBox;
}

void ANFSluiceZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANFSluiceZone, bGateClosed);
	DOREPLIFETIME(ANFSluiceZone, HeldWaterZ);
}

void ANFSluiceZone::BeginPlay()
{
	Super::BeginPlay();

	if (UNFTideSubsystem* Subsystem = GetWorld()->GetSubsystem<UNFTideSubsystem>())
	{
		Subsystem->RegisterLocalWaterModifier(this);
	}
}

void ANFSluiceZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HoldTimerHandle);
		if (UNFTideSubsystem* Subsystem = World->GetSubsystem<UNFTideSubsystem>())
		{
			Subsystem->UnregisterLocalWaterModifier(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

bool ANFSluiceZone::GetLocalWaterCap(const FVector& WorldLocation, float& OutMaxWaterZ) const
{
	if (!bGateClosed || !ZoneBox)
	{
		return false;
	}

	// XY test in the box's own space, so rotated zones work. Z is ignored: the zone is a column.
	const FVector LocalPosition = ZoneBox->GetComponentTransform().InverseTransformPosition(WorldLocation);
	const FVector Extent = ZoneBox->GetUnscaledBoxExtent();
	if (FMath::Abs(LocalPosition.X) > Extent.X || FMath::Abs(LocalPosition.Y) > Extent.Y)
	{
		return false;
	}

	OutMaxWaterZ = HeldWaterZ;
	return true;
}

void ANFSluiceZone::CloseGate()
{
	if (!HasAuthority() || bGateClosed)
	{
		return;
	}

	const ANFTideManager* TideManager = ANFTideManager::Get(GetWorld());
	if (!TideManager)
	{
		return;
	}

	HeldWaterZ = TideManager->GetGlobalWaterZ();
	bGateClosed = true;
	ForceNetUpdate();

	GetWorldTimerManager().SetTimer(HoldTimerHandle, this, &ANFSluiceZone::OpenGate, FMath::Max(HoldSeconds, 0.1f), false);
}

void ANFSluiceZone::OpenGate()
{
	if (!HasAuthority() || !bGateClosed)
	{
		return;
	}

	GetWorldTimerManager().ClearTimer(HoldTimerHandle);
	bGateClosed = false;
	ForceNetUpdate();
}
