#include "NFHealthComponent.h"
#include "NewFPS.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

namespace NFHealth
{
	/** Seconds between regeneration steps. */
	const float RegenInterval = 0.25f;
}

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
	if (const UWorld* World = GetWorld())
	{
		LastDamageTime = World->GetTimeSeconds();
	}
	OnHealthChanged.Broadcast(this, Health, -FinalDamage, DamageInfo.Instigator);
	OnDamageTaken.Broadcast(DamageInfo, FinalDamage);

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

void UNFHealthComponent::ReviveTo(float NewHealth)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	const float OldHealth = Health;
	Health = FMath::Clamp(NewHealth, 1.f, MaxHealth);

	if (bIsDead)
	{
		bIsDead = false;
		OnRevived.Broadcast(this);
	}

	OnHealthChanged.Broadcast(this, Health, Health - OldHealth, nullptr);
}

void UNFHealthComponent::Kill(AController* Killer)
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || bIsDead)
	{
		return;
	}

	const float OldHealth = Health;
	Health = 0.f;
	bIsDead = true;
	OnHealthChanged.Broadcast(this, Health, -OldHealth, Killer);
	OnDeath.Broadcast(this, Killer);
}

void UNFHealthComponent::SetMaxHealth(float NewMaxHealth, bool bRefill)
{
	// The maximum is plain data every machine can set; the current health is the server's.
	MaxHealth = FMath::Max(NewMaxHealth, 1.f);

	const AActor* Owner = GetOwner();
	if (Owner && Owner->HasAuthority())
	{
		Health = bRefill ? MaxHealth : FMath::Min(Health, MaxHealth);
	}
}

void UNFHealthComponent::SetInvulnerableFor(float Seconds)
{
	UWorld* World = GetWorld();
	if (!World || Seconds <= 0.f)
	{
		return;
	}

	bInvulnerable = true;
	World->GetTimerManager().SetTimer(InvulnerableTimerHandle, this, &UNFHealthComponent::EndTimedInvulnerability, Seconds, false);
}

void UNFHealthComponent::EndTimedInvulnerability()
{
	bInvulnerable = false;
}

void UNFHealthComponent::ConfigureRegen(float DelaySeconds, float RatePerSecond, float CapHealth)
{
	RegenDelay = FMath::Max(DelaySeconds, 0.f);
	RegenRate = FMath::Max(RatePerSecond, 0.f);
	RegenCap = CapHealth;
}

void UNFHealthComponent::SetRegenEnabled(bool bEnabled)
{
	const AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !Owner->HasAuthority() || !World || bRegenEnabled == bEnabled)
	{
		return;
	}

	bRegenEnabled = bEnabled;
	if (bEnabled)
	{
		World->GetTimerManager().SetTimer(RegenTimerHandle, this, &UNFHealthComponent::RegenTick, NFHealth::RegenInterval, true);
	}
	else
	{
		World->GetTimerManager().ClearTimer(RegenTimerHandle);
	}
}

void UNFHealthComponent::RegenTick()
{
	const UWorld* World = GetWorld();
	if (!World || bIsDead || RegenRate <= 0.f)
	{
		return;
	}

	const float Cap = RegenCap > 0.f ? FMath::Min(RegenCap, MaxHealth) : MaxHealth;
	if (Health >= Cap || World->GetTimeSeconds() - LastDamageTime < RegenDelay)
	{
		return;
	}

	const float OldHealth = Health;
	Health = FMath::Min(Health + RegenRate * NFHealth::RegenInterval, Cap);
	OnHealthChanged.Broadcast(this, Health, Health - OldHealth, nullptr);
}

void UNFHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RegenTimerHandle);
		World->GetTimerManager().ClearTimer(InvulnerableTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
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
