#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NFTypes.h"
#include "NFEnemyBase.generated.h"

class AAIController;
class UMaterialInstanceDynamic;
class UNFEnemyDirector;
class UNFHealthComponent;
class UNFWetnessComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;

/** What an enemy is doing, as far as presentation cares. Replicated; drive poses, lamp colours and sounds from it. */
UENUM(BlueprintType)
enum class ENFEnemyVisualState : uint8
{
	Idle,
	Moving,
	/** Wind-up of an attack. AttackId says which one (see the enemy class). */
	Telegraph,
	Attacking,
	Recovering,
	Staggered,
	/** Exposed: Scuttler after a missed lunge, Attendant being serviced. */
	Vulnerable,
	Dead
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNFOnEnemyVisualStateChanged, ENFEnemyVisualState, NewState, uint8, AttackId);

/**
 * Common enemy body for the prototype: health, wetness, target choice, damage reactions, the replicated visual
 * state, and a C++ state machine hook (ServerThink) driven by a plain AAIController's NavMesh MoveTo.
 * There is deliberately no Behavior Tree asset: the prototype has to run without any .uasset.
 *
 * Everything that decides runs on the server. Clients only get movement (ACharacter replication), health and
 * the visual state.
 */
UCLASS(Abstract)
class NEWFPS_API ANFEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	ANFEnemyBase();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Enemy")
	UNFHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool IsAlive() const;

	UFUNCTION(BlueprintPure, Category = "Enemy")
	ENFEnemyVisualState GetVisualState() const { return VisualState; }

	/** Which attack the Telegraph/Attacking state belongs to (enemy specific; 0 = none). */
	UFUNCTION(BlueprintPure, Category = "Enemy")
	int32 GetVisualAttackId() const { return VisualAttackId; }

	/** Server only: shove this enemy about DistanceCm away from FromLocation. */
	void ApplyKnockback(const FVector& FromLocation, float DistanceCm);

	/**
	 * Server only: launch any character so that it travels about DistanceCm along Direction (flat) before it
	 * lands. Uses LaunchCharacter, so it works on players too (the launch is replicated by the engine).
	 */
	static void LaunchByDistance(ACharacter* Victim, const FVector& Direction, float DistanceCm);

	/** Every machine: the visual state changed. Bind sounds/animation here, or use the Blueprint event below. */
	UPROPERTY(BlueprintAssignable, Category = "Enemy")
	FNFOnEnemyVisualStateChanged OnVisualStateChanged;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Server only, every frame while alive: the enemy's state machine. */
	virtual void ServerThink(float DeltaSeconds) {}

	/** Server only: a hit landed and the enemy survived it. */
	virtual void OnDamaged(const FNFDamageInfo& DamageInfo, float AppliedDamage) {}

	/** Server only: just died. Tokens are already released. */
	virtual void OnDied() {}

	/** Every machine except a dedicated server: bring the grey-box visuals in line with the replicated state. */
	virtual void RefreshVisuals() {}

	/** Blueprint hook for artists: same moment as OnVisualStateChanged. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy", meta = (DisplayName = "On Visual State Changed"))
	void K2_OnVisualStateChanged(ENFEnemyVisualState NewState, int32 AttackId);

	UFUNCTION()
	void OnRep_VisualState();

	UFUNCTION()
	void HandleDeath(UNFHealthComponent* InHealthComponent, AController* Killer);

	// ---- Helpers for subclasses (server) ----

	void SetVisualState(ENFEnemyVisualState NewState, uint8 AttackId = 0);
	void NotifyVisualStateChanged();

	AAIController* GetAI() const;
	UNFEnemyDirector* GetDirector() const;

	/** Water depth at this enemy's own feet (cm); very negative when the level has no tide. */
	float GetOwnWaterDepth() const;

	/** A living, standing player pawn. Downed players are never targets (spec 5). */
	bool IsValidTarget(const APawn* Candidate) const;

	/** Nearest valid player, or null. */
	APawn* FindNearestTarget() const;

	/** Keeps CurrentTarget fresh (re-evaluated twice a second, or at once when it became invalid). */
	void UpdateTarget();

	float GetDistance2DTo(const AActor* Other) const;
	float GetWorldTime() const;

	void MoveToTargetActor(AActor* Goal, float AcceptanceRadius);
	void MoveToPoint(const FVector& Goal, float AcceptanceRadius);
	void StopMoving();
	bool IsMoving() const;

	/** Damage a player through the single damage pipeline. Returns the damage applied. */
	float DealDamage(AActor* Victim, float Amount, ENFDamageType DamageType);

	/** Turn a primitive into a weapon-trace-only hit volume (optionally the weak point). */
	static void SetupHitVolume(UPrimitiveComponent* Component, bool bWeakPoint);

	/** Turn a static mesh component into a collision-free grey-box visual. */
	static void SetupVisual(UStaticMeshComponent* Component);

	/** Set the "Color" parameter of the engine basic shape material. */
	static void SetVisualColor(UMaterialInstanceDynamic* Material, const FLinearColor& Color);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNFHealthComponent> HealthComponent;

	/** Evaluated locally on every machine; subclasses apply their own thresholds to GetWaterDepth. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNFWetnessComponent> WetnessComponent;

	/** Seconds the body stays after death before the actor is removed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "0.1", Units = "s"))
	float CorpseSeconds = 1.5f;

	/** Upward launch speed of every knockback (cm/s). Higher = a longer, floatier shove. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy", meta = (ClampMin = "50.0"))
	float KnockbackUpSpeed = 200.f;

	TWeakObjectPtr<APawn> CurrentTarget;

private:
	void HandleDamageTakenNative(const FNFDamageInfo& DamageInfo, float AppliedDamage);

	UPROPERTY(ReplicatedUsing = OnRep_VisualState)
	ENFEnemyVisualState VisualState = ENFEnemyVisualState::Idle;

	UPROPERTY(ReplicatedUsing = OnRep_VisualState)
	uint8 VisualAttackId = 0;

	float NextTargetSearchTime = 0.f;
	bool bDeathHandled = false;
};
