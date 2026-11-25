#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "NFTypes.h"
#include "NFWeaponComponent.generated.h"

class APawn;
class APlayerController;
class UMeshComponent;
class UNFWeaponData;
class USoundBase;
struct FStreamableHandle;

/** One shot as the owning client saw it. Sent to the server for validation. */
USTRUCT()
struct FNFShotData
{
	GENERATED_BODY()

	/** Camera location the trace started from. */
	UPROPERTY()
	FVector_NetQuantize Origin;

	/** Final shot direction (spread already applied). */
	UPROPERTY()
	FVector_NetQuantizeNormal Direction;

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

	/** Wrapping shot counter; also the low byte of the spread seed. */
	UPROPERTY()
	uint8 ShotId = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNFOnFired, FVector, TraceEnd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNFOnAmmoChanged, int32, AmmoInMagazine, int32, ReserveAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FNFOnHitConfirmed, AActor*, HitActor, float, Damage, ENFHitZone, Zone, bool, bKilled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNFOnReloadStateChanged, bool, bIsReloading);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNFOnDryFire);
DECLARE_MULTICAST_DELEGATE(FNFOnWeaponVisualsReady);

/**
 * Hitscan weapon logic for a pawn.
 *
 * Network model: the owning client decides when a shot happens, traces from its camera, plays cosmetics
 * immediately and predicts ammo. It sends the shot (unreliable) to the server, which validates fire rate,
 * ammo, origin, range and line of sight before applying damage through UNFHealthComponent. There is no
 * server rewind yet: the server accepts the client's hit claim inside tolerances (see the Validation properties).
 */
UCLASS(ClassGroup = (NewFPS), meta = (BlueprintSpawnableComponent))
class NEWFPS_API UNFWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
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

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetAiming(bool bNewAiming);

	/**
	 * Tell the weapon how much the player moved the view this frame (degrees; pitch positive = up).
	 * Input that fights the recoil is not "given back" by recoil recovery.
	 */
	void NotifyAimInput(float YawDelta, float PitchDelta);

	// ---- State ----

	/** Never null: falls back to the UNFWeaponData class defaults when no asset is assigned. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	const UNFWeaponData* GetData() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetAmmoInMagazine() const { return AmmoInMagazine; }

	/** -1 when the weapon has infinite reserve. */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetReserveAmmo() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsReloading() const { return bReloading; }

	/** Wants to aim and is allowed to (not reloading). */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsAiming() const { return bAiming && !bReloading; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool IsTriggerHeld() const { return bTriggerHeld; }

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

	/** Owning client: the server confirmed that a shot damaged something. */
	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FNFOnHitConfirmed OnHitConfirmed;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FNFOnReloadStateChanged OnReloadStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FNFOnDryFire OnDryFire;

	/** Soft-referenced meshes/sounds/VFX of the weapon data finished loading. */
	FNFOnWeaponVisualsReady OnVisualsReady;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(Server, Unreliable)
	void ServerFire(const FNFShotData& Shot);

	UFUNCTION(Server, Reliable)
	void ServerStartReload();

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bNewAiming);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFireFX(const FVector_NetQuantize& TraceEnd, const FVector_NetQuantizeNormal& ImpactNormal, bool bBlockingHit);

	UFUNCTION(Client, Unreliable)
	void ClientConfirmHit(AActor* HitActor, float Damage, ENFHitZone Zone, bool bKilled);

	UFUNCTION()
	void OnRep_Reloading();

	/** Weapon definition. Leave empty to use the class defaults of UNFWeaponData. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UNFWeaponData> WeaponData;

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

	/** The server finishes a remote client's reload this much earlier, so the client's first shot after reload is not rejected. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Validation", meta = (ClampMin = "0.0"))
	float ServerReloadLeniency = 0.25f;

	/** Owning client: seconds after the last predicted ammo change before the server's ammo count is adopted. */
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Validation", meta = (ClampMin = "0.0"))
	float AmmoReconcileDelay = 0.6f;

private:
	APawn* GetOwnerPawn() const;
	bool IsOwnerLocallyControlled() const;
	bool OwnerHasAuthority() const;
	bool IsOwnerDead() const;
	bool GetAimView(FVector& OutLocation, FRotator& OutRotation) const;

	bool CanFire() const;
	bool CanReload() const;
	bool WantsShot() const;
	void UpdateFiring(float DeltaTime);
	void HandleCannotFire();
	void FireShot();
	void AuthProcessShot(const FNFShotData& Shot, bool bFromRemoteClient);
	bool AuthValidateHitClaim(const FNFShotData& Shot, const APawn* OwnerPawn, const UNFWeaponData* Data) const;

	void BeginReload();
	void FinishReload();
	void SetAmmo(int32 NewMagazine, int32 NewReserve);

	void AddRecoil(FRandomStream& Stream);
	void UpdateRecoil(float DeltaTime);
	void ApplyViewDelta(float YawDelta, float PitchDelta) const;

	void PlayFireFX(const FVector& TraceEnd, const FVector& ImpactNormal, bool bBlockingHit);
	void PlaySoundAtMuzzle(const TSoftObjectPtr<USoundBase>& Sound) const;
	void RequestVisualAssets();
	void HandleVisualAssetsLoaded();

	// Authoritative ammo, replicated to the owner only. The owner predicts into AmmoInMagazine/ReserveAmmo.
	UPROPERTY(Replicated)
	int32 RepAmmoInMagazine = 0;

	UPROPERTY(Replicated)
	int32 RepReserveAmmo = 0;

	// Owner sets these locally (prediction); other clients get them from the server.
	UPROPERTY(Replicated)
	bool bAiming = false;

	UPROPERTY(ReplicatedUsing = OnRep_Reloading)
	bool bReloading = false;

	UPROPERTY(Transient)
	TObjectPtr<UMeshComponent> VisualMesh;

	int32 AmmoInMagazine = 0;
	int32 ReserveAmmo = 0;

	bool bTriggerHeld = false;
	bool bSemiShotPending = false;
	bool bDryFiredThisPress = false;
	float LastFirePressTime = -1000.f;
	float ShotCooldown = 0.f;
	float LastShotTime = -1000.f;
	float LastPredictedAmmoChangeTime = -1000.f;
	int32 ShotCounter = 0;
	int32 BurstShotIndex = 0;
	float SpreadHeat = 0.f;
	float AdsAlpha = 0.f;

	/** Kick not yet fed into the camera (X = yaw, Y = pitch up). */
	FVector2D PendingRecoil = FVector2D::ZeroVector;
	/** Kick already in the camera that recovery may still give back. */
	FVector2D RecoverableRecoil = FVector2D::ZeroVector;

	/** Server: earliest time the next shot is allowed (token bucket). */
	float ServerNextFireTime = 0.f;

	FTimerHandle ReloadTimerHandle;
	TSharedPtr<FStreamableHandle> VisualAssetsHandle;
};
