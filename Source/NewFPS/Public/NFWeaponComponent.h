#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "NFTypes.h"
#include "NFWeaponComponent.generated.h"

class APawn;
class APlayerController;
class UMeshComponent;
class UNFMeleeData;
class UNFTideData;
class UNFWeaponData;
class UNFWetnessComponent;
class USoundBase;
struct FStreamableHandle;

/** One projectile of a shot as the owning client saw it. A rifle shot has one, a shotgun shot several. */
USTRUCT()
struct FNFPelletData
{
	GENERATED_BODY()

	/** Where the trace ended (impact point, or max range on a miss). */
	UPROPERTY()
	FVector_NetQuantize TraceEnd;

	UPROPERTY()
	FVector_NetQuantizeNormal ImpactNormal;

	/** Actor the client claims to have hit (null on a miss or world hit without an actor). */
	UPROPERTY()
	TObjectPtr<AActor> HitActor = nullptr;

	UPROPERTY()
	ENFHitZone HitZone = ENFHitZone::Body;

	UPROPERTY()
	bool bBlockingHit = false;
};

/** One trigger pull as the owning client saw it. Sent to the server for validation. */
USTRUCT()
struct FNFShotData
{
	GENERATED_BODY()

	/** Camera location the traces started from. */
	UPROPERTY()
	FVector_NetQuantize Origin;

	/** Aim direction (centre of the spread cone). */
	UPROPERTY()
	FVector_NetQuantizeNormal Direction;

	UPROPERTY()
	TArray<FNFPelletData> Pellets;

	/** Wrapping shot counter; also the low byte of the spread seed. */
	UPROPERTY()
	uint8 ShotId = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNFOnFired, FVector, TraceEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNFOnAmmoChanged, int32, AmmoInMagazine, int32, ReserveAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FNFOnHitConfirmed, AActor*, HitActor, float, Damage, ENFHitZone, Zone, bool, bKilled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNFOnReloadStateChanged, bool, bIsReloading);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNFOnDryFire);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNFOnWeaponSwitched, int32, NewSlot);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNFOnMeleeStarted);
DECLARE_MULTICAST_DELEGATE(FNFOnWeaponVisualsReady);

/** What the wielder's situation (water depth, carrying) allows a firearm to do right now. */
struct FNFWeaponUse
{
	/** Can be fired at all. */
	bool bUsable = true;
	/** ADS is not available (waist deep, or one hand on a strongbox). */
	bool bHipOnly = false;
	bool bCanReload = true;
	float SpreadScale = 1.f;
	float ReloadTimeScale = 1.f;
};

/**
 * Hitscan weapon logic for a pawn: two firearm slots, weapon switching, reload (cancellable) and the Kedge melee.
 *
 * Network model: the owning client decides when a shot happens, traces from its camera, plays cosmetics
 * immediately and predicts ammo. It sends the shot (unreliable) to the server, which validates fire rate,
 * ammo, weapon state, origin, range, spread cone and line of sight before applying damage through
 * UNFHealthComponent. There is no server rewind: the server accepts the client's hit claim inside tolerances
 * (see the Validation properties). Melee is different: the client only asks, the server finds the target.
 *
 * Water and carrying rules are not replicated flags: every machine derives them from the owner's water depth
 * (UNFWetnessComponent, deterministic) and from the carry state the character mirrors into SetCarrying.
 */
UCLASS(ClassGroup = (NewFPS), meta = (BlueprintSpawnableComponent))
class NEWFPS_API UNFWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/** Purser and Gaffer. The Kedge is not a slot. */
	static constexpr int32 NumSlots = 2;

	UNFWeaponComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ---- Input (call on the owning client) ----

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartReload();

	/** Abort a running reload: progress is lost, no ammo is given, it does not restart by itself. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void CancelReload();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetAiming(bool bNewAiming);

	/** Raise the other firearm (takes the new weapon's EquipTime; cancels a reload). */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SwitchWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SelectSlot(int32 Slot);

	/** Swing the Kedge. Cancels a reload; firing and ADS stay locked for the swing interval. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void StartMelee();

	/**
	 * Tell the weapon how much the player moved the view this frame (degrees; pitch positive = up).
	 * Input that fights the recoil is not "given back" by recoil recovery.
	 */
	void NotifyAimInput(float YawDelta, float PitchDelta);

	// ---- State ----

	/** Data of the weapon in hand. Never null: an empty Loadout entry falls back to the spec weapon of that slot. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	const UNFWeaponData* GetData() const;

	/** Never null. Slot 0 defaults to UNFPurserData, slot 1 to UNFGafferData. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	const UNFWeaponData* GetSlotData(int32 Slot) const;

	/** Never null: falls back to the UNFMeleeData class defaults. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	const UNFMeleeData* GetMeleeData() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetActiveSlot() const { return ActiveSlot; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetAmmoInMagazine() const { return AmmoInMagazine; }

	/** -1 when the weapon has infinite reserve. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetReserveAmmo() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsReloading() const { return bReloading; }

	/** Wants to aim and is allowed to (not reloading/switching/swinging, not waist deep, not carrying). */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsAiming() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsTriggerHeld() const { return bTriggerHeld; }

	/** The new weapon is still coming up. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsSwitching() const;

	/** Inside the swing interval of the Kedge. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsMeleeActive() const;

	/** The firearm in hand cannot be fired because of the water depth or the strongbox. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsFirearmUnusable() const;

	/** Water depth / carrying limits for a weapon right now. DepthSlack (cm) loosens the depth test (server validation). */
	FNFWeaponUse EvaluateUse(const UNFWeaponData* Data, float DepthSlack = 0.f) const;

	/**
	 * External lock for everything this component does (e.g. downed). Not replicated: the caller sets it on every
	 * machine from state both sides know. A blocked weapon neither fires nor accepts remote shots.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetFireBlocked(bool bNewBlocked);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsFireBlocked() const { return bFireBlocked; }

	/**
	 * Mirror of "the owner carries a strongbox". Set by the character on every machine. While carrying only
	 * weapons with bUsableWhileCarrying work, hip-fire only, no reload, no Kedge.
	 */
	void SetCarrying(bool bNewCarrying);

	bool IsCarrying() const { return bCarrying; }

	/** Server only: if the weapon in hand is unusable right now but the other one is not, raise the other one. */
	void AuthEnsureUsableSlot();

	/** Server only: add reserve ammo to a slot (ammo crate). Amount < 0 refills to the starting reserve. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void AuthAddReserveAmmo(int32 Slot, int32 Amount);

	/** 0 = hip, 1 = fully aimed. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetAdsAlpha() const { return AdsAlpha; }

	/** Current spread cone half-angle in degrees. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	float GetCurrentSpreadDegrees() const;

	/** Look sensitivity multiplier for the current ADS state, given the hip FOV. */
	float GetLookSensitivityScale(float HipFOV) const;

	/** FOV the camera should use right now. */
	float GetDesiredFOV(float HipFOV) const;

	/** The mesh that carries the muzzle socket (first-person weapon mesh). Set by the owning character. */
	void SetVisualMesh(UMeshComponent* InMesh) { VisualMesh = InMesh; }

	/** Muzzle world location for cosmetics. Falls back to an offset from the view when there is no mesh/socket. */
	FVector GetMuzzleLocation() const;

	// ---- Events ----

	/** Owning client / listen host: a shot left the weapon (fires immediately, before server confirmation). */
	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FNFOnFired OnFired;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FNFOnAmmoChanged OnAmmoChanged;

	/** Owning client: the server confirmed that a shot or swing damaged something. */
	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FNFOnHitConfirmed OnHitConfirmed;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FNFOnReloadStateChanged OnReloadStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FNFOnDryFire OnDryFire;

	/** Every machine: another firearm is coming up (start the equip animation). */
	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FNFOnWeaponSwitched OnWeaponSwitched;

	/** Every machine: a Kedge swing started (the hit lands UNFMeleeData::HitDelay later, on the server). */
	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FNFOnMeleeStarted OnMeleeStarted;

	/** Soft-referenced meshes/sounds/VFX of the weapon in hand finished loading. */
	FNFOnWeaponVisualsReady OnVisualsReady;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(Server, Unreliable)
	void ServerFire(const FNFShotData& Shot);

	UFUNCTION(Server, Reliable)
	void ServerStartReload();

	UFUNCTION(Server, Reliable)
	void ServerCancelReload();

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bNewAiming);

	UFUNCTION(Server, Reliable)
	void ServerSelectSlot(uint8 Slot);

	UFUNCTION(Server, Reliable)
	void ServerStartMelee();

	/** BlockingMask: bit N set = pellet N hit something. */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFireFX(const TArray<FVector_NetQuantize>& TraceEnds, uint8 BlockingMask);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastMeleeFX();

	UFUNCTION(Client, Unreliable)
	void ClientConfirmHit(AActor* HitActor, float Damage, ENFHitZone Zone, bool bKilled);

	UFUNCTION()
	void OnRep_Reloading();

	UFUNCTION()
	void OnRep_ActiveSlot();

	/**
	 * The two firearms: index 0 and 1. An empty array or an empty entry uses the spec weapon of that slot
	 * (0 = UNFPurserData, 1 = UNFGafferData class defaults), so no asset is needed to try the prototype.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TArray<TObjectPtr<UNFWeaponData>> Loadout;

	/** The Kedge. Leave empty to use the class defaults of UNFMeleeData. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UNFMeleeData> MeleeData;

	/** Off (spec 6.1): shots and swings do not damage other players. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	bool bFriendlyFire = false;

	/** Muzzle offset from the view (forward, right, up) used when no mesh/socket is available. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FVector FallbackMuzzleOffset = FVector(60.f, 18.f, -14.f);

	// ---- Server validation tolerances ----

	/** Seconds of fire-rate slack for packet bunching. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Validation", meta = (ClampMin = "0.0"))
	float ServerFireRateTolerance = 0.15f;

	/** Max distance (cm) between the client's shot origin and the server's eye position. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Validation", meta = (ClampMin = "0.0"))
	float ServerMaxOriginError = 250.f;

	/** The claimed impact must be inside the target's bounds expanded by this (cm). Covers target movement during latency. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Validation", meta = (ClampMin = "0.0"))
	float ServerHitBoundsTolerance = 150.f;

	/** Degrees a pellet may leave the weapon's widest possible spread cone before the server drops it. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Validation", meta = (ClampMin = "0.0"))
	float ServerSpreadTolerance = 2.f;

	/** Water depth slack (cm) for remote clients: their depth sample is up to one update interval and one latency old. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Validation", meta = (ClampMin = "0.0"))
	float ServerDepthTolerance = 15.f;

	/**
	 * The server ends a remote client's timed states (reload, weapon switch, swing lock) this much earlier,
	 * so the client's first shot afterwards is not rejected.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Validation", meta = (ClampMin = "0.0"))
	float ServerReloadLeniency = 0.25f;

	/** Owning client: seconds after the last predicted ammo/slot change before the server's value is adopted. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Validation", meta = (ClampMin = "0.0"))
	float AmmoReconcileDelay = 0.6f;

private:
	APawn* GetOwnerPawn() const;
	bool IsOwnerLocallyControlled() const;
	bool OwnerHasAuthority() const;
	bool IsOwnerDead() const;
	bool GetAimView(FVector& OutLocation, FRotator& OutRotation) const;
	float GetWorldTime() const;
	float GetOwnerWaterDepth() const;
	const UNFTideData* GetTideRules() const;
	bool IsFriendlyTarget(const AActor* Target) const;

	bool CanFire() const;
	bool CanReload() const;
	bool CanMelee(float DepthSlack) const;
	bool WantsShot() const;
	void UpdateFiring(float DeltaTime);
	void HandleCannotFire();
	void FireShot();
	void AuthProcessShot(const FNFShotData& Shot, bool bFromRemoteClient);
	bool AuthValidatePellet(const FNFShotData& Shot, const FNFPelletData& Pellet, const APawn* OwnerPawn, const UNFWeaponData* Data) const;

	void BeginReload();
	void FinishReload();
	void CancelReloadInternal();
	void SetAmmo(int32 NewMagazine, int32 NewReserve);

	/** Put Slot in hand on this machine: stash/restore ammo, start the equip time, reload the visuals. */
	void ApplySlot(int32 Slot);

	void BeginMeleeLocal();
	void AuthStartMelee();
	void AuthMeleeHit();

	void AddRecoil(FRandomStream& Stream);
	void UpdateRecoil(float DeltaTime);
	void ApplyViewDelta(float YawDelta, float PitchDelta) const;

	void PlayFireFX(const TArray<FVector>& TraceEnds, const TArray<FVector>& ImpactNormals, uint32 BlockingMask);
	void PlaySoundAtMuzzle(const TSoftObjectPtr<USoundBase>& Sound) const;
	void RequestVisualAssets();
	void HandleVisualAssetsLoaded();

	// Authoritative ammo of the weapon in hand, replicated to the owner only. The owner predicts into AmmoInMagazine/ReserveAmmo.
	UPROPERTY(Replicated)
	int32 RepAmmoInMagazine = 0;

	UPROPERTY(Replicated)
	int32 RepReserveAmmo = 0;

	/** Authoritative slot. The owner predicts into ActiveSlot and adopts this after AmmoReconcileDelay. */
	UPROPERTY(ReplicatedUsing = OnRep_ActiveSlot)
	uint8 RepActiveSlot = 0;

	// Owner sets these locally (prediction); other clients get them from the server.
	UPROPERTY(Replicated)
	bool bAiming = false;

	UPROPERTY(ReplicatedUsing = OnRep_Reloading)
	bool bReloading = false;

	UPROPERTY(Transient)
	TObjectPtr<UMeshComponent> VisualMesh;

	UPROPERTY(Transient)
	TObjectPtr<UNFWetnessComponent> OwnerWetness;

	int32 ActiveSlot = 0;
	int32 AmmoInMagazine = 0;
	int32 ReserveAmmo = 0;

	/** Ammo of each slot while it is not in hand (the slot in hand lives in AmmoInMagazine/ReserveAmmo). */
	int32 StoredMagazine[NumSlots] = { 0, 0 };
	int32 StoredReserve[NumSlots] = { 0, 0 };

	bool bFireBlocked = false;
	bool bCarrying = false;
	bool bTriggerHeld = false;
	bool bSemiShotPending = false;
	bool bDryFiredThisPress = false;
	float LastFirePressTime = -1000.f;
	float ShotCooldown = 0.f;
	float LastShotTime = -1000.f;
	float LastPredictedAmmoChangeTime = -1000.f;
	float LastSlotRequestTime = -1000.f;
	int32 ShotCounter = 0;
	int32 BurstShotIndex = 0;
	float SpreadHeat = 0.f;
	float AdsAlpha = 0.f;

	/** World time the weapon coming up is ready. */
	float SwitchEndTime = -1000.f;
	/** World time the current swing's fire/ADS lock ends; also the earliest time of the next swing. */
	float MeleeEndTime = -1000.f;

	/** Kick not yet fed into the camera (X = yaw, Y = pitch up). */
	FVector2D PendingRecoil = FVector2D::ZeroVector;
	/** Kick already in the camera that recovery may still give back. */
	FVector2D RecoverableRecoil = FVector2D::ZeroVector;

	/** Server: earliest time the next shot is allowed (token bucket). */
	float ServerNextFireTime = 0.f;
	/** Server: earliest time the next swing of a remote client is allowed. */
	float ServerNextMeleeTime = 0.f;

	FTimerHandle ReloadTimerHandle;
	FTimerHandle MeleeHitTimerHandle;
	TSharedPtr<FStreamableHandle> VisualAssetsHandle;
};
