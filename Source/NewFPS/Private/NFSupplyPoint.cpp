#include "NFSupplyPoint.h"
#include "NewFPS.h"
#include "NFCharacter.h"
#include "NFHealthComponent.h"
#include "NFRulesData.h"
#include "NFWeaponComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

namespace NFSupplyConst
{
	/** Colour parameter of /Engine/BasicShapes/BasicShapeMaterial (same as the grey-box enemies). */
	const FName ColorParameter(TEXT("Color"));
	/** Placeholder box size (cm). */
	const FVector BoxSize(60.f, 40.f, 30.f);
}

ANFSupplyPoint::ANFSupplyPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	// Two to four per level, and the use count is what a player decides on from a distance.
	bAlwaysRelevant = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
	BoxMesh->SetupAttachment(SceneRoot);
	// Engine cube is 100 cm; the actor origin is the floor.
	BoxMesh->SetRelativeScale3D(NFSupplyConst::BoxSize / 100.f);
	BoxMesh->SetRelativeLocation(FVector(0.f, 0.f, NFSupplyConst::BoxSize.Z * 0.5f));
	// Seen, but it never blocks movement, bullets or the NavMesh (same as the strongbox).
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

void ANFSupplyPoint::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANFSupplyPoint, UsesConsumed);
}

void ANFSupplyPoint::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() != NM_DedicatedServer)
	{
		BoxMaterial = BoxMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (BoxMaterial)
		{
			BoxMaterial->SetVectorParameterValue(NFSupplyConst::ColorParameter, SupplyType == ENFSupplyType::Ammo ? AmmoColor : FirstAidColor);
		}
	}
}

int32 ANFSupplyPoint::GetRemainingUsesFor(const ANFCharacter* User) const
{
	if (SupplyType == ENFSupplyType::Ammo)
	{
		return -1;
	}

	// Spec 6.1: uses per box = players in the session. Read live, so a late joiner brings a use along.
	const UNFRulesData* Rules = User ? User->GetRules() : GetDefault<UNFRulesData>();
	const UWorld* World = GetWorld();
	const AGameStateBase* SessionState = World ? World->GetGameState() : nullptr;
	const int32 NumPlayers = SessionState ? FMath::Max(SessionState->PlayerArray.Num(), 1) : 1;
	return FMath::Max(NumPlayers * Rules->FirstAidUsesPerPlayer - UsesConsumed, 0);
}

float ANFSupplyPoint::GetHoldSecondsFor(const ANFCharacter* User) const
{
	const UNFRulesData* Rules = User ? User->GetRules() : GetDefault<UNFRulesData>();
	return SupplyType == ENFSupplyType::Ammo ? Rules->AmmoCrateHoldSeconds : Rules->FirstAidHoldSeconds;
}

bool ANFSupplyPoint::CanBeUsedBy(const ANFCharacter* User) const
{
	if (!User || User->GetLifeState() != ENFLifeState::Alive)
	{
		return false;
	}

	if (SupplyType == ENFSupplyType::Ammo)
	{
		// Unlimited and idempotent (it refills to the starting reserve), so it is always worth a hold.
		return User->GetWeaponComponent() != nullptr;
	}

	// A use is not spent on a player it would do nothing for.
	const UNFHealthComponent* UserHealth = User->GetHealthComponent();
	return UserHealth
		&& GetRemainingUsesFor(User) > 0
		&& UserHealth->GetHealth() < FMath::Min(User->GetRules()->FirstAidHealth, UserHealth->GetMaxHealth());
}

bool ANFSupplyPoint::AuthUse(ANFCharacter* User)
{
	if (!HasAuthority() || !CanBeUsedBy(User))
	{
		return false;
	}

	if (SupplyType == ENFSupplyType::Ammo)
	{
		// Amount < 0 = up to the weapon's starting reserve. Weapons with an infinite reserve ignore it.
		UNFWeaponComponent* UserWeapon = User->GetWeaponComponent();
		for (int32 Slot = 0; Slot < UNFWeaponComponent::NumSlots; ++Slot)
		{
			UserWeapon->AuthAddReserveAmmo(Slot, -1);
		}
		UE_LOG(LogNewFPS, Log, TEXT("Supply: %s restocked ammo at %s"), *GetNameSafe(User), *GetName());
		return true;
	}

	// The player is alive, so this only sets the health (clamped to the maximum) and tells the listeners.
	User->GetHealthComponent()->ReviveTo(User->GetRules()->FirstAidHealth);
	++UsesConsumed;
	ForceNetUpdate();
	UE_LOG(LogNewFPS, Log, TEXT("Supply: %s used first aid at %s (%d left)"), *GetNameSafe(User), *GetName(), GetRemainingUsesFor(User));
	return true;
}
