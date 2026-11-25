#include "NFStrongbox.h"
#include "NewFPS.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

ANFStrongbox::ANFStrongbox()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	// A handful per mission, and the objective of the whole team: everyone always knows where each one is.
	bAlwaysRelevant = true;
	SetReplicateMovement(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
	RootComponent = BoxMesh;
	BoxMesh->SetMobility(EComponentMobility::Movable);
	// Engine cube is 100 cm.
	BoxMesh->SetRelativeScale3D(BoxSize / 100.f);
	// Seen and traced for, but it never blocks movement, bullets or the NavMesh.
	BoxMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	BoxMesh->SetGenerateOverlapEvents(false);
	BoxMesh->SetCanEverAffectNavigation(false);
	if (CubeMesh.Succeeded())
	{
		BoxMesh->SetStaticMesh(CubeMesh.Object);
	}
}

void ANFStrongbox::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANFStrongbox, Carrier);
	DOREPLIFETIME(ANFStrongbox, bDelivered);
}

void ANFStrongbox::BeginPlay()
{
	Super::BeginPlay();

	// Late joiners get the state before BeginPlay.
	ApplyState();
}

void ANFStrongbox::AuthSetCarrier(AActor* NewCarrier)
{
	if (!HasAuthority() || Carrier == NewCarrier)
	{
		return;
	}

	Carrier = NewCarrier;
	ApplyState();
	ForceNetUpdate();
}

void ANFStrongbox::AuthDropAt(const FVector& WorldLocation, const FRotator& WorldRotation)
{
	if (!HasAuthority())
	{
		return;
	}

	SetActorLocationAndRotation(WorldLocation, WorldRotation, false, nullptr, ETeleportType::TeleportPhysics);
	Carrier = nullptr;
	ApplyState();
	ForceNetUpdate();
}

void ANFStrongbox::AuthMarkDelivered()
{
	if (!HasAuthority() || bDelivered)
	{
		return;
	}

	bDelivered = true;
	ApplyState();
	ForceNetUpdate();
}

void ANFStrongbox::OnRep_State()
{
	ApplyState();
}

void ANFStrongbox::ApplyState()
{
	const bool bCarried = Carrier != nullptr;
	SetActorHiddenInGame(bCarried);
	SetActorEnableCollision(!bCarried);
}
