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

	static const FName HeadTag;
	static const FName LimbTag;

protected:
	virtual void BeginPlay() override;

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
};
