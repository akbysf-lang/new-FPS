#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Curves/CurveFloat.h"
#include "NFTypes.h"
#include "NFWeaponData.generated.h"

class USkeletalMesh;
class UStaticMesh;
class USoundBase;
class UNiagaraSystem;

/**
 * All designer-tunable numbers and presentation references for one weapon.
 * Create in the editor: Content Browser -> Miscellaneous -> Data Asset -> NFWeaponData.
 * The class defaults are a usable 600 RPM rifle so the prototype runs without any asset.
 */
UCLASS(BlueprintType)
class NEWFPS_API UNFWeaponData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UNFWeaponData();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** Seconds between shots. */
	float GetFireInterval() const { return 60.f / FMath::Max(RoundsPerMinute, 1.f); }

	/** Damage multiplier at a distance in cm (curve is authored in meters). */
	float GetDistanceMultiplier(float DistanceCm) const;

	float GetZoneMultiplier(ENFHitZone Zone) const;

	/** Camera kick for the Nth shot of a burst (X = yaw right, Y = pitch up, degrees), without randomness. */
	FVector2D GetRecoilForShot(int32 ShotIndex) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	FText DisplayName;

	// ---- Damage ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
	float Damage = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
	float HeadshotMultiplier = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
	float LimbMultiplier = 0.85f;

	/** Max hitscan range in meters. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "1.0"))
	float RangeMeters = 150.f;

	/** X = distance in meters, Y = damage multiplier. Empty curve = no falloff. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	FRuntimeFloatCurve DamageFalloffCurve;

	// ---- Fire ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire", meta = (ClampMin = "1.0"))
	float RoundsPerMinute = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire")
	bool bFullAuto = true;

	/** Semi-auto only: a press this many seconds before the weapon is ready still fires. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fire", meta = (ClampMin = "0.0"))
	float FireInputBuffer = 0.1f;

	// ---- Ammo ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "1"))
	int32 MagazineSize = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	bool bInfiniteReserve = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0", EditCondition = "!bInfiniteReserve"))
	int32 StartingReserveAmmo = 120;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo", meta = (ClampMin = "0.05"))
	float ReloadTime = 2.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	bool bAutoReloadOnEmpty = true;

	// ---- Spread (half-angle, degrees) ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float HipSpread = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float AdsSpread = 0.1f;

	/** Added at MoveSpreadReferenceSpeed, scaled linearly below it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float MoveSpread = 1.f;

	/** Ground speed (cm/s) at which the full MoveSpread applies. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "1.0"))
	float MoveSpreadReferenceSpeed = 600.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float AirborneSpread = 3.f;

	/** Bloom added per consecutive shot. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float SpreadPerShot = 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float MaxBloom = 2.5f;

	/** Bloom "shots" forgotten per second. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float SpreadRecoveryPerSecond = 8.f;

	/** Multiplier on move/airborne/bloom spread while fully aimed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spread", meta = (ClampMin = "0.0"))
	float AdsSpreadModifier = 0.3f;

	// ---- Recoil (camera) ----

	/** Per-shot camera kick in degrees. X = yaw (positive = right), Y = pitch (positive = up). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil")
	TArray<FVector2D> RecoilPattern;

	/** When the burst runs past the pattern it loops from this index. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0"))
	int32 RecoilLoopStartIndex = 4;

	/** Random +/- degrees added to each kick axis. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float RecoilRandomness = 0.08f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float AdsRecoilMultiplier = 0.7f;

	/** How fast a kick is fed into the camera (1/s). Higher = snappier. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "1.0"))
	float RecoilKickSpeed = 30.f;

	/** Seconds after the last shot before the camera starts to return. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float RecoilRecoveryDelay = 0.12f;

	/** Camera return speed in degrees per second. 0 = no recovery. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float RecoilRecoverySpeed = 12.f;

	/** Seconds without firing before the pattern restarts at shot 0. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recoil", meta = (ClampMin = "0.0"))
	float RecoilResetTime = 0.35f;

	// ---- ADS ----

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ADS", meta = (ClampMin = "10.0", ClampMax = "170.0"))
	float AdsFOV = 65.f;

	/** Seconds to go from hip to fully aimed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ADS", meta = (ClampMin = "0.01"))
	float AdsTime = 0.2f;

	/** Extra multiplier on top of the automatic FOV-ratio sensitivity scaling. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ADS", meta = (ClampMin = "0.05"))
	float AdsSensitivityScale = 1.f;

	// ---- Presentation (soft references; loaded async when the weapon is equipped) ----

	/** Use this OR WeaponStaticMesh. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TSoftObjectPtr<USkeletalMesh> WeaponSkeletalMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	TSoftObjectPtr<UStaticMesh> WeaponStaticMesh;

	/** Offset of the weapon mesh relative to the socket it is attached to. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	FTransform WeaponAttachOffset;

	/** Socket on the weapon mesh the tracer / muzzle flash come from. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
	FName MuzzleSocketName = TEXT("Muzzle");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation|Sound")
	TSoftObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation|Sound")
	TSoftObjectPtr<USoundBase> DryFireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation|Sound")
	TSoftObjectPtr<USoundBase> ReloadSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation|Sound")
	TSoftObjectPtr<USoundBase> HitConfirmSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation|Sound")
	TSoftObjectPtr<USoundBase> HeadshotConfirmSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation|Sound")
	TSoftObjectPtr<USoundBase> KillConfirmSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation|VFX")
	TSoftObjectPtr<UNiagaraSystem> MuzzleFlashFX;

	/** Spawned at the muzzle facing the impact; TracerEndParameter receives the world-space end point. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation|VFX")
	TSoftObjectPtr<UNiagaraSystem> TracerFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation|VFX")
	FName TracerEndParameter = TEXT("BeamEnd");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation|VFX")
	TSoftObjectPtr<UNiagaraSystem> ImpactFX;
};
