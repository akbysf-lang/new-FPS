#include "NFWinchPoint.h"
#include "NFGameState.h"
#include "NFStrongbox.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

ANFWinchPoint::ANFWinchPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// Put the actor origin on the platform floor: the volume reaches 200 cm up from it.
	NetVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("NetVolume"));
	NetVolume->SetupAttachment(SceneRoot);
	NetVolume->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	NetVolume->SetBoxExtent(FVector(200.f, 200.f, 100.f));
	NetVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NetVolume->SetGenerateOverlapEvents(false);
	NetVolume->SetCanEverAffectNavigation(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	NetMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NetMarker"));
	NetMarker->SetupAttachment(SceneRoot);
	NetMarker->SetRelativeLocation(FVector(0.f, 0.f, 2.f));
	// Engine plane is 100 cm.
	NetMarker->SetRelativeScale3D(FVector(4.f, 4.f, 1.f));
	NetMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NetMarker->SetGenerateOverlapEvents(false);
	NetMarker->SetCanEverAffectNavigation(false);
	NetMarker->SetCastShadow(false);
	if (PlaneMesh.Succeeded())
	{
		NetMarker->SetStaticMesh(PlaneMesh.Object);
	}
}

bool ANFWinchPoint::IsInsideNet(const FVector& WorldLocation) const
{
	if (!NetVolume)
	{
		return false;
	}

	// Tested in the box's own space, so a rotated or scaled volume works.
	const FVector LocalPosition = NetVolume->GetComponentTransform().InverseTransformPosition(WorldLocation);
	const FVector Extent = NetVolume->GetUnscaledBoxExtent();
	return FMath::Abs(LocalPosition.X) <= Extent.X
		&& FMath::Abs(LocalPosition.Y) <= Extent.Y
		&& FMath::Abs(LocalPosition.Z) <= Extent.Z;
}

bool ANFWinchPoint::AuthTryDeliver(ANFStrongbox* Strongbox)
{
	if (!HasAuthority() || !Strongbox || Strongbox->IsDelivered() || Strongbox->GetCarrier() != nullptr)
	{
		return false;
	}
	if (!IsInsideNet(Strongbox->GetActorLocation()))
	{
		return false;
	}

	Strongbox->AuthMarkDelivered();

	if (ANFGameState* GameState = GetWorld()->GetGameState<ANFGameState>())
	{
		GameState->AuthAddDeliveredStrongbox();
	}
	return true;
}
