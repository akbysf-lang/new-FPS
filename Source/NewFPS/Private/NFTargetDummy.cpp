#include "NFTargetDummy.h"
#include "NFHealthComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace NFTargetDummy
{
	/** Colour parameter of /Engine/BasicShapes/BasicShapeMaterial. */
	const FName ColorParameter(TEXT("Color"));
}

ANFTargetDummy::ANFTargetDummy()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// Engine basic shapes are 100 cm across. Body: 50 cm wide, 140 cm tall, standing on the actor origin.
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(SceneRoot);
	BodyMesh->SetMobility(EComponentMobility::Movable);
	BodyMesh->SetRelativeLocation(FVector(0.f, 0.f, 70.f));
	BodyMesh->SetRelativeScale3D(FVector(0.5f, 0.5f, 1.4f));
	BodyMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	if (CylinderMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(CylinderMesh.Object);
	}

	// Head: 32 cm sphere on top of the body.
	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(SceneRoot);
	HeadMesh->SetMobility(EComponentMobility::Movable);
	HeadMesh->SetRelativeLocation(FVector(0.f, 0.f, 157.f));
	HeadMesh->SetRelativeScale3D(FVector(0.32f));
	HeadMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	HeadMesh->ComponentTags.Add(UNFHealthComponent::HeadTag);
	if (SphereMesh.Succeeded())
	{
		HeadMesh->SetStaticMesh(SphereMesh.Object);
	}

	HealthComponent = CreateDefaultSubobject<UNFHealthComponent>(TEXT("HealthComponent"));
}

void ANFTargetDummy::BeginPlay()
{
	Super::BeginPlay();

	HealthComponent->OnHealthChanged.AddUniqueDynamic(this, &ANFTargetDummy::HandleHealthChanged);
	HealthComponent->OnDeath.AddUniqueDynamic(this, &ANFTargetDummy::HandleDeath);
	HealthComponent->OnRevived.AddUniqueDynamic(this, &ANFTargetDummy::HandleRevived);

	if (GetNetMode() != NM_DedicatedServer)
	{
		BodyMaterial = BodyMesh->CreateAndSetMaterialInstanceDynamic(0);
		HeadMaterial = HeadMesh->CreateAndSetMaterialInstanceDynamic(0);
		ApplyHitReact(0.f);
	}

	// A client that joins while the target is dead gets bIsDead before BeginPlay binds the delegates.
	if (HealthComponent->IsDead())
	{
		HandleDeath(HealthComponent, nullptr);
	}
}

void ANFTargetDummy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	HitReactTimeRemaining = FMath::Max(0.f, HitReactTimeRemaining - DeltaSeconds);
	ApplyHitReact(HitReactTimeRemaining / FMath::Max(HitReactDuration, KINDA_SMALL_NUMBER));

	if (HitReactTimeRemaining <= 0.f)
	{
		SetActorTickEnabled(false);
	}
}

void ANFTargetDummy::ApplyHitReact(float Alpha)
{
	const FLinearColor Color = FMath::Lerp(BaseColor, HitColor, Alpha);
	if (BodyMaterial)
	{
		BodyMaterial->SetVectorParameterValue(NFTargetDummy::ColorParameter, Color);
	}
	if (HeadMaterial)
	{
		HeadMaterial->SetVectorParameterValue(NFTargetDummy::ColorParameter, Color);
	}

	// Purely visual pulse; the dedicated server keeps the collision shapes untouched.
	if (GetNetMode() != NM_DedicatedServer)
	{
		const float Punch = 1.f + HitReactScalePunch * Alpha;
		BodyMesh->SetRelativeScale3D(FVector(0.5f * Punch, 0.5f * Punch, 1.4f));
		HeadMesh->SetRelativeScale3D(FVector(0.32f * Punch));
	}
}

void ANFTargetDummy::HandleHealthChanged(UNFHealthComponent* InHealthComponent, float NewHealth, float Delta, AController* InstigatedBy)
{
	if (Delta < 0.f)
	{
		HitReactTimeRemaining = HitReactDuration;
		SetActorTickEnabled(true);
	}
}

void ANFTargetDummy::HandleDeath(UNFHealthComponent* InHealthComponent, AController* Killer)
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ANFTargetDummy::Respawn, RespawnDelay, false);
	}
}

void ANFTargetDummy::Respawn()
{
	HealthComponent->Revive();
}

void ANFTargetDummy::HandleRevived(UNFHealthComponent* InHealthComponent)
{
	HitReactTimeRemaining = 0.f;
	ApplyHitReact(0.f);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}
