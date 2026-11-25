#pragma once

#include "CoreMinimal.h"
#include "NFEnemyBase.h"
#include "NFAttendant.generated.h"

class UCapsuleComponent;
class UMaterialInstanceDynamic;
class UNFAttendantData;
class USphereComponent;
class UStaticMeshComponent;

/**
 * Attendant, the Wardens' basic ranged automaton (prototype-spec.md 5.2).
 *
 *  - Punch Burst (AttackId 1): 0.7 s telegraph (lamp red), then 3 slow projectiles 0.15 s apart, 2 s cooldown.
 *    The first burst after it notices a player misses on purpose. Needs one of the global fire tokens.
 *  - Service window: after 4 bursts it stands exposed for 3 s (visual state Vulnerable, lamp blinks at 1 Hz).
 *  - Shove (AttackId 2): player within 150 cm -> 0.4 s telegraph, 8 damage + a 300 cm push, 0.5 s recovery.
 *  - Water: slowed from knee depth, shorts out (dies) after 2 s at waist depth. Spec 5.2.1 K1-K4: never picks
 *    a move goal standing AvoidDepth (40) deep, walks out at once when its own feet are that deep, and walks to
 *    higher ground (floor to floor +50, a random one of the highest spots) when the rising water gets within
 *    150 cm of its feet - except on the top level (floor Z >= 750), where it stays.
 *
 * Capsule 40 / 95. Hit volumes: a body capsule (Body) and the lamp sphere, radius 18 with its centre 172 cm
 * above the floor (weak point).
 *
 * Not implemented in the prototype: taking cover (it repositions to a random reachable spot instead), kneeling
 * behind low cover, patrol routes, barks, a NavMesh path cost for water (K5: a path through water is caught by
 * the escape rule instead).
 */
UCLASS()
class NEWFPS_API ANFAttendant : public ANFEnemyBase
{
	GENERATED_BODY()

public:
	ANFAttendant();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Never null: falls back to the UNFAttendantData class defaults. */
	UFUNCTION(BlueprintPure, Category = "Attendant")
	const UNFAttendantData* GetData() const;

	/** Knee deep or deeper: slowed and sparking (push it further in). */
	UFUNCTION(BlueprintPure, Category = "Attendant")
	bool IsSparking() const { return bSparking; }

	static constexpr uint8 AttackPunchBurst = 1;
	static constexpr uint8 AttackShove = 2;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void ServerThink(float DeltaSeconds) override;
	virtual void OnDamaged(const FNFDamageInfo& DamageInfo, float AppliedDamage) override;
	virtual void RefreshVisuals() override;

	UFUNCTION()
	void OnRep_Sparking();

	/** Numbers. Leave empty to use the class defaults of UNFAttendantData (the spec's starting values). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attendant")
	TObjectPtr<UNFAttendantData> Data;

	/** Projectiles leave from here (relative to the capsule centre). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attendant")
	FVector MuzzleOffset = FVector(50.f, 0.f, 45.f);

	// Hit volumes (weapon traces only).
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCapsuleComponent> BodyHit;

	/** Weak point: the signal lamp. Radius 18, centre 172 cm above the floor (Z 154-190, inside the capsule top). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> LampHit;

	// Grey-box visuals (engine basic shapes). Replace with the real mesh in a Blueprint.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BodyVisual;

	/** The lamp is the telegraph: calm cream, red = fire incoming, slow blink = being serviced. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> LampVisual;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attendant|Presentation")
	FLinearColor LampCalmColor = FLinearColor(1.f, 0.957f, 0.839f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attendant|Presentation")
	FLinearColor LampAlertColor = FLinearColor(1.f, 0.04f, 0.02f);

private:
	enum class EThinkState : uint8
	{
		Idle,
		/** In or moving into the preferred range; waiting for a fire token and the cooldown. */
		Engage,
		BurstTelegraph,
		BurstFiring,
		Service,
		ShoveTelegraph,
		ShoveRecover,
		Stagger,
		Retreat
	};

	void SetThinkState(EThinkState NewState);
	/** Returns false when the water killed it. */
	bool UpdateWater(float DeltaSeconds);
	void ThinkEngage(APawn* Target, float Distance);
	bool TryStartRetreat();
	void TryReposition(APawn* Target);
	/** World Z of the floor under its feet (capsule bottom). All water rules compare floor to floor (spec 5.2.1). */
	float GetOwnFloorZ() const;
	/** K1: would it stand AvoidDepth or deeper at this NavMesh point? */
	bool IsFloorTooDeep(const FVector& FloorPoint) const;
	void FireProjectile(APawn* Target, bool bMissOnPurpose);
	void ReleaseFireToken();
	void BlinkLamp();

	UPROPERTY(ReplicatedUsing = OnRep_Sparking)
	bool bSparking = false;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> LampMaterial;

	EThinkState ThinkState = EThinkState::Idle;
	float StateTime = 0.f;
	float WaistSeconds = 0.f;
	float LastWaterZ = 0.f;
	float NextWaterCheckTime = 0.f;
	bool bWaterRising = false;
	float NextBurstTime = 0.f;
	float NextShotTime = 0.f;
	float NextShoveTime = 0.f;
	float NextMoveOrderTime = 0.f;
	float NextRepositionTime = 0.f;
	float NextRetreatSearchTime = 0.f;
	int32 ShotsFiredInBurst = 0;
	int32 BurstsSinceService = 0;
	bool bHasFireToken = false;
	/** The next burst is the first one after noticing a player. */
	bool bFirstBurstPending = true;
	float MissSide = 1.f;
	bool bLampBlinkOn = false;

	FTimerHandle LampBlinkTimerHandle;
};
