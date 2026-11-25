#pragma once

#include "CoreMinimal.h"
#include "NFEnemyBase.h"
#include "NFScuttler.generated.h"

class UBoxComponent;
class UMaterialInstanceDynamic;
class UNFScuttlerData;
class USphereComponent;
class UStaticMeshComponent;

/** How the water has left a Scuttler. Replicated for presentation. */
UENUM(BlueprintType)
enum class ENFScuttlerWaterState : uint8
{
	/** In the water: fast, heals. */
	Wet,
	Dry,
	/** Dry for 20 s: slow, takes x1.5 damage. */
	Parched
};

/**
 * Scuttler, the Drift's melee swarm (prototype-spec.md 5.1): fast and healing in the water, slow on dry ground,
 * brittle once parched. Two attacks with telegraphs: Pinch (AttackId 1) and, wet only, Lunge (AttackId 2).
 * Only AttackersPerTarget of them attack one player at a time; the rest circle.
 *
 * Capsule 35 / 35. Hit volumes: shell box and two claws (Body), the sac on its back (weak point). No legs:
 * they take no hits and have no collision.
 */
UCLASS()
class NEWFPS_API ANFScuttler : public ANFEnemyBase
{
	GENERATED_BODY()

public:
	ANFScuttler();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Never null: falls back to the UNFScuttlerData class defaults. */
	UFUNCTION(BlueprintPure, Category = "Scuttler")
	const UNFScuttlerData* GetData() const;

	UFUNCTION(BlueprintPure, Category = "Scuttler")
	ENFScuttlerWaterState GetWaterState() const { return WaterState; }

	static constexpr uint8 AttackPinch = 1;
	static constexpr uint8 AttackLunge = 2;

protected:
	virtual void BeginPlay() override;
	virtual void ServerThink(float DeltaSeconds) override;
	virtual void OnDamaged(const FNFDamageInfo& DamageInfo, float AppliedDamage) override;
	virtual void RefreshVisuals() override;

	UFUNCTION()
	void OnRep_WaterState();

	/** Numbers. Leave empty to use the class defaults of UNFScuttlerData (the spec's starting values). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scuttler")
	TObjectPtr<UNFScuttlerData> Data;

	// Hit volumes (weapon traces only).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> ShellHit;

	/** Weak point: about 25 cm across, on the back, seen from behind and above. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> SacHit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> ClawHitLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> ClawHitRight;

	// Grey-box visuals (engine basic shapes). Replace with the real mesh in a Blueprint.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ShellVisual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SacVisual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ClawVisualLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ClawVisualRight;

private:
	enum class EThinkState : uint8
	{
		Idle,
		Chase,
		Circle,
		PinchTelegraph,
		PinchRecover,
		LungeTelegraph,
		LungeLeap,
		LungeRecover,
		Stagger
	};

	void SetThinkState(EThinkState NewState);
	void UpdateWater(float DeltaSeconds);
	bool TryTakeAttackToken(APawn* Target);
	void ReleaseAttackToken();
	void ThinkChase(APawn* Target, float Distance);
	void ThinkCircle(APawn* Target, float Distance);

	UPROPERTY(ReplicatedUsing = OnRep_WaterState)
	ENFScuttlerWaterState WaterState = ENFScuttlerWaterState::Dry;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ShellMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> SacMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ClawMaterialLeft;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> ClawMaterialRight;

	EThinkState ThinkState = EThinkState::Idle;
	float StateTime = 0.f;
	float DrySeconds = 0.f;
	float NextAttackTime = 0.f;
	float NextMoveOrderTime = 0.f;
	float NextTokenRequestTime = 0.f;
	float RecoverDuration = 0.f;
	float CircleDirection = 1.f;
	bool bHasAttackToken = false;
	bool bLungeHit = false;
};
