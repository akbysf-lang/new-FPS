#include "NFHealthComponent.h"
#include "NewFPS.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Net/UnrealNetwork.h"

const FName UNFHealthComponent::HeadTag(TEXT("NF.Head"));
const FName UNFHealthComponent::LimbTag(TEXT("NF.Limb"));

UNFHealthComponent::UNFHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// UE mannequin naming; replace per skeleton in the owning Blueprint.
	HeadBones = { TEXT("head"), TEXT("neck_01"), TEXT("neck_02") };
	LimbBones = {
		TEXT("upperarm_l"), TEXT("lowerarm_l"), TEXT("hand_l"),
		TEXT("upperarm_r"), TEXT("lowerarm_r"), TEXT("hand_r"),
		TEXT("thigh_l"), TEXT("calf_l"), TEXT("foot_l"),
		TEXT("thigh_r"), TEXT("calf_r"), TEXT("foot_r")
	};
}

void UNFHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UNFHealthComponent, Health);
	DOREPLIFETIME(UNFHealthComponent, bIsDead);
}

void UNFHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Health = MaxHealth;
	}
}

float UNFHealthComponent::ApplyDamage(const FNFDamageInfo& DamageInfo)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || bIsDead || bInvulnerable)
	{
		return 0.f;
	}

	const float RawDamage = DamageInfo.BaseDamage * DamageInfo.DistanceMultiplier * DamageInfo.ZoneMultiplier * DamageTakenMultiplier;
	const float FinalDamage = FMath::Clamp(RawDamage, 0.f, Health);

	UE_LOG(LogNewFPS, Verbose, TEXT("Damage -> %s: base=%.1f dist=x%.2f zone=%d x%.2f resist=x%.2f final=%.1f (hp %.1f -> %.1f) by %s"),
		*GetNameSafe(Owner), DamageInfo.BaseDamage, DamageInfo.DistanceMultiplier, static_cast<int32>(DamageInfo.Zone),
		DamageInfo.ZoneMultiplier, DamageTakenMultiplier, FinalDamage, Health, Health - FinalDamage, *GetNameSafe(DamageInfo.Instigator));

	if (FinalDamage <= 0.f)
	{
		return 0.f;
	}

	Health -= FinalDamage;
	OnHealthChanged.Broadcast(this, Health, -FinalDamage, DamageInfo.Instigator);

	if (Health <= 0.f)
	{
		Health = 0.f;
		bIsDead = true;
		OnDeath.Broadcast(this, DamageInfo.Instigator);
	}

	return FinalDamage;
}

void UNFHealthComponent::Revive()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	const float OldHealth = Health;
	Health = MaxHealth;

	if (bIsDead)
	{
		bIsDead = false;
		OnRevived.Broadcast(this);
	}

	OnHealthChanged.Broadcast(this, Health, Health - OldHealth, nullptr);
}

void UNFHealthComponent::OnRep_Health(float OldHealth)
{
	OnHealthChanged.Broadcast(this, Health, Health - OldHealth, nullptr);
}

void UNFHealthComponent::OnRep_IsDead()
{
	if (bIsDead)
	{
		OnDeath.Broadcast(this, nullptr);
	}
	else
	{
		OnRevived.Broadcast(this);
	}
}

UNFHealthComponent* UNFHealthComponent::FindHealthComponent(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UNFHealthComponent>() : nullptr;
}

ENFHitZone UNFHealthComponent::ResolveHitZone(const FHitResult& Hit)
{
	if (const UPrimitiveComponent* HitComponent = Hit.GetComponent())
	{
		if (HitComponent->ComponentHasTag(HeadTag))
		{
			return ENFHitZone::Head;
		}
		if (HitComponent->ComponentHasTag(LimbTag))
		{
			return ENFHitZone::Limb;
		}
	}

	if (!Hit.BoneName.IsNone())
	{
		if (const UNFHealthComponent* HealthComponent = FindHealthComponent(Hit.GetActor()))
		{
			if (HealthComponent->HeadBones.Contains(Hit.BoneName))
			{
				return ENFHitZone::Head;
			}
			if (HealthComponent->LimbBones.Contains(Hit.BoneName))
			{
				return ENFHitZone::Limb;
			}
		}
	}

	return ENFHitZone::Body;
}
