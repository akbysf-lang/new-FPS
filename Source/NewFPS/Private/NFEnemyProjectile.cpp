#include "NFEnemyProjectile.h"
#include "NFHealthComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/HitResult.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

ANFEnemyProjectile::ANFEnemyProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);
	// Max range 35 m at 35 m/s is one second of flight.
	InitialLifeSpan = 2.f;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	RootComponent = Collision;
	Collision->InitSphereRadius(8.f);
	// Stops at the world, passes through pawns as an overlap (no physical shove), ignores everything else.
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Collision->SetGenerateOverlapEvents(true);
	Collision->SetCanEverAffectNavigation(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Collision);
	Visual->SetRelativeScale3D(FVector(0.16f));
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetGenerateOverlapEvents(false);
	Visual->SetCastShadow(false);
	if (SphereMesh.Succeeded())
	{
		Visual->SetStaticMesh(SphereMesh.Object);
	}

	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->SetUpdatedComponent(Collision);
	Movement->InitialSpeed = 3500.f;
	Movement->MaxSpeed = 3500.f;
	Movement->ProjectileGravityScale = 0.f;
	Movement->bRotationFollowsVelocity = true;
	Movement->bShouldBounce = false;
}

void ANFEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Shooter = GetInstigator())
	{
		Collision->IgnoreActorWhenMoving(Shooter, true);
	}

	// Hits are resolved on the server only; clients just watch the replicated actor fly.
	if (HasAuthority())
	{
		Collision->OnComponentBeginOverlap.AddDynamic(this, &ANFEnemyProjectile::HandleOverlap);
		Movement->OnProjectileStop.AddDynamic(this, &ANFEnemyProjectile::HandleStop);
	}
}

void ANFEnemyProjectile::InitProjectile(float InDamage, float InSpeed)
{
	Damage = InDamage;
	if (InSpeed > 0.f && Movement)
	{
		Movement->MaxSpeed = InSpeed;
		Movement->Velocity = GetActorForwardVector() * InSpeed;
	}
}

void ANFEnemyProjectile::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bSpent || !OtherActor || OtherActor == GetInstigator())
	{
		return;
	}

	// Only players are hurt; it flies through other enemies. Downed players are "dead" for the health component.
	const APawn* OtherPawn = Cast<APawn>(OtherActor);
	UNFHealthComponent* Health = UNFHealthComponent::FindHealthComponent(OtherActor);
	if (!OtherPawn || !OtherPawn->IsPlayerControlled() || !Health || Health->IsDead())
	{
		return;
	}

	bSpent = true;

	FNFDamageInfo DamageInfo;
	DamageInfo.BaseDamage = Damage;
	DamageInfo.DamageType = ENFDamageType::EnemyProjectile;
	DamageInfo.HitLocation = GetActorLocation();
	DamageInfo.ShotDirection = GetActorForwardVector();
	DamageInfo.Instigator = GetInstigatorController();
	DamageInfo.Causer = GetInstigator();
	Health->ApplyDamage(DamageInfo);

	Destroy();
}

void ANFEnemyProjectile::HandleStop(const FHitResult& ImpactResult)
{
	if (!bSpent)
	{
		bSpent = true;
		Destroy();
	}
}
