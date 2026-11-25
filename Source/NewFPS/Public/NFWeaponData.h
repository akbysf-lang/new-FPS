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
 * The class defaults are a generic 600 RPM test rifle. The prototype's two firearms have their spec values as
 * class defaults in the subclasses below (UNFPurserData, UNFGafferData): create a Data Asset of one of those
 * classes to start from the spec numbers, or leave UNFWeaponComponent::Loadout empty to use them as they are.
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

	/** Per pellet when PelletsPerShot > 1. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "0.0"))
	float Damage = 25.f;

	/**
	 * Projectiles per trigger pull (shotgun pellets). Each pellet is traced independently inside the spread cone
	 * and gets its own tracer; the damage of all pellets that hit the same target is applied as ONE hit, so the
	 * target sees the total of the shot (stagger thresholds). One pull always costs one round.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ClampMin = "1", ClampMax = "16"))
	int32 PelletsPerShot = 1;

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

	// ---- Handling ----

	/** Seconds from selecting this weapon until it can fire (weapon switch time). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Handling", meta = (ClampMin = "0.0"))
	float EquipTime = 0.4f;

	/** Can be fired one-handed while carrying a strongbox (hip-fire only, no reload). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Handling")
	bool bUsableWhileCarrying = false;

	/**
	 * Deepest wetness state this weapon still works in. The lock itself is bound to depth: the weapon stops at the
	 * threshold of the next state (Knee -> stops at WaistDepth; Waist -> would stop at OverheadDepth), and never
	 * works at UNFTideData::ChestDepth or deeper. Gaffer: Knee. Purser: Waist.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Handling")
	ENFWetness MaxUsableWetness = ENFWetness::Waist;

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

/** Purser, the break-top revolver (prototype-spec.md 4.1). Class defaults = spec starting values. */
UCLASS(BlueprintType)
class NEWFPS_API UNFPurserData : public UNFWeaponData
{
	GENERATED_BODY()

public:
	UNFPurserData();
};

/** Gaffer, the pump shotgun (prototype-spec.md 4.2). Class defaults = spec starting values. */
UCLASS(BlueprintType)
class NEWFPS_API UNFGafferData : public UNFWeaponData
{
	GENERATED_BODY()

public:
	UNFGafferData();
};

/**
 * Kedge, the boat-hook melee everyone carries (prototype-spec.md 4.3). Not a weapon slot: swung with its own
 * input while the firearm stays in hand. Class defaults = spec starting values.
 */
UCLASS(BlueprintType)
class NEWFPS_API UNFMeleeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(FPrimaryAssetType(TEXT("NFMelee")), GetFName());
	}

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Info")
	FText DisplayName;

	/** No weak-point multiplier. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee", meta = (ClampMin = "0.0"))
	float Damage = 40.f;

	/** Seconds between swings; firing and ADS are locked for this long after the button. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee", meta = (ClampMin = "0.05"))
	float SwingInterval = 0.6f;

	/** Seconds from the button to the hit. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee", meta = (ClampMin = "0.0"))
	float HitDelay = 0.15f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee", meta = (ClampMin = "10.0", Units = "cm"))
	float Reach = 180.f;

	/** Full cone angle in degrees; the nearest target inside it is the only one hit. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee", meta = (ClampMin = "1.0", ClampMax = "180.0"))
	float ConeAngle = 60.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation|Sound")
	TSoftObjectPtr<USoundBase> SwingSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation|Sound")
	TSoftObjectPtr<USoundBase> HitSound;
};
