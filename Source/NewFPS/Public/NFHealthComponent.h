#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "NFTypes.h"
#include "NFHealthComponent.generated.h"

class UNFHealthComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FNFOnHealthChanged, UNFHealthComponent*, HealthComponent, float, NewHealth, float, Delta, AController*, InstigatedBy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNFOnDeath, UNFHealthComponent*, HealthComponent, AController*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNFOnRevived, UNFHealthComponent*, HealthComponent);
/** Server only, native: the full damage record of one hit and the damage that was actually applied. */
DECLARE_MULTICAST_DELEGATE_TwoParams(FNFOnDamageTakenNative, const FNFDamageInfo& /*DamageInfo*/, float /*AppliedDamage*/);

/**
 * Replicated health. All damage in the game goes through ApplyDamage (server only).
 * Delegates fire on the server and, through OnRep, on clients (InstigatedBy/Killer are null on clients).
 */
UCLASS(ClassGroup = (NewFPS), meta = (BlueprintSpawnableComponent))
class NEWFPS_API UNFHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNFHealthComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * The single damage pipeline: base -> distance falloff -> zone multiplier -> resistance -> final.
	 * Server only. Returns the damage actually applied.
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float ApplyDamage(const FNFDamageInfo& DamageInfo);

	/** Server only: restore full health and clear the dead state. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Revive();

	/** Server only: set health to NewHealth (clamped to 1..MaxHealth) and clear the dead state. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ReviveTo(float NewHealth);

	/** Server only: drop to 0 and die, ignoring resistances and invulnerability (short circuit, kill rules). */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Kill(AController* Killer);

	/** Any machine sets the maximum; only the server touches the current health (bRefill sets it to the new maximum). */
	void SetMaxHealth(float NewMaxHealth, bool bRefill);

	void SetDamageTakenMultiplier(float NewMultiplier) { DamageTakenMultiplier = FMath::Max(NewMultiplier, 0.f); }

	/** Server only: ignore damage for Seconds (revive grace). */
	void SetInvulnerableFor(float Seconds);

	/**
	 * Server only. After DelaySeconds without damage, heal RatePerSecond up to CapHealth (<= 0 means MaxHealth).
	 * Configure once, then switch on and off with SetRegenEnabled.
	 */
	void ConfigureRegen(float DelaySeconds, float RatePerSecond, float CapHealth);
	void SetRegenEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category = "Health")
	static UNFHealthComponent* FindHealthComponent(const AActor* Actor);

	/**
	 * Resolve the hit region. Order: component tag "NF.Head"/"NF.Limb", then bone name against
	 * the hit actor's health component bone lists. Anything else is Body.
	 */
	UFUNCTION(BlueprintPure, Category = "Health")
	static ENFHitZone ResolveHitZone(const FHitResult& Hit);

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FNFOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FNFOnDeath OnDeath;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FNFOnRevived OnRevived;

	/** Server only. Fires for every hit that applied damage, before OnDeath. */
	FNFOnDamageTakenNative OnDamageTaken;

	static const FName HeadTag;
	static const FName LimbTag;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UFUNCTION()
	void OnRep_IsDead();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.f;

	/** Resistance/armor stage of the pipeline. 1 = none, 0.5 = takes half damage. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "0.0"))
	float DamageTakenMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	bool bInvulnerable = false;

	/** Skeletal bones counted as head hits. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health|Zones")
	TArray<FName> HeadBones;

	/** Skeletal bones counted as limb hits. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health|Zones")
	TArray<FName> LimbBones;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleInstanceOnly, BlueprintReadOnly, Category = "Health")
	float Health = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_IsDead, VisibleInstanceOnly, BlueprintReadOnly, Category = "Health")
	bool bIsDead = false;

private:
	void RegenTick();
	void EndTimedInvulnerability();

	float RegenDelay = 0.f;
	float RegenRate = 0.f;
	float RegenCap = 0.f;
	bool bRegenEnabled = false;
	float LastDamageTime = -1000.f;

	FTimerHandle RegenTimerHandle;
	FTimerHandle InvulnerableTimerHandle;
};
