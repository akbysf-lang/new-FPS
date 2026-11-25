#include "NFWeaponData.h"
#include "Sound/SoundBase.h"

UNFWeaponData::UNFWeaponData()
{
	DisplayName = FText::FromString(TEXT("Test Rifle"));

	// Full damage to 25 m, 60% at 50 m and beyond.
	FRichCurve* Falloff = DamageFalloffCurve.GetRichCurve();
	Falloff->AddKey(0.f, 1.f);
	Falloff->AddKey(25.f, 1.f);
	Falloff->AddKey(50.f, 0.6f);

	// Climb first, then a left/right weave that loops (see RecoilLoopStartIndex).
	RecoilPattern = {
		FVector2D(0.00f, 0.60f),
		FVector2D(0.05f, 0.55f),
		FVector2D(-0.05f, 0.50f),
		FVector2D(0.10f, 0.45f),
		FVector2D(0.20f, 0.30f),
		FVector2D(0.25f, 0.25f),
		FVector2D(-0.15f, 0.25f),
		FVector2D(-0.30f, 0.20f),
		FVector2D(-0.20f, 0.25f),
		FVector2D(0.15f, 0.20f)
	};
}

FPrimaryAssetId UNFWeaponData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType(TEXT("NFWeapon")), GetFName());
}

float UNFWeaponData::GetDistanceMultiplier(float DistanceCm) const
{
	const FRichCurve* Falloff = DamageFalloffCurve.GetRichCurveConst();
	if (!Falloff || Falloff->GetNumKeys() == 0)
	{
		return 1.f;
	}
	return FMath::Max(Falloff->Eval(DistanceCm * 0.01f), 0.f);
}

float UNFWeaponData::GetZoneMultiplier(ENFHitZone Zone) const
{
	switch (Zone)
	{
	case ENFHitZone::Head:
		return HeadshotMultiplier;
	case ENFHitZone::Limb:
		return LimbMultiplier;
	default:
		return 1.f;
	}
}

FVector2D UNFWeaponData::GetRecoilForShot(int32 ShotIndex) const
{
	const int32 Num = RecoilPattern.Num();
	if (Num == 0 || ShotIndex < 0)
	{
		return FVector2D::ZeroVector;
	}
	if (ShotIndex < Num)
	{
		return RecoilPattern[ShotIndex];
	}

	const int32 LoopStart = FMath::Clamp(RecoilLoopStartIndex, 0, Num - 1);
	const int32 LoopLength = Num - LoopStart;
	return RecoilPattern[LoopStart + ((ShotIndex - Num) % LoopLength)];
}

// ---------------------------------------------------------------------------------------------
// Spec weapons. Every number below is a starting value from prototype-spec.md section 4.
// ---------------------------------------------------------------------------------------------

UNFPurserData::UNFPurserData()
{
	DisplayName = FText::FromString(TEXT("Purser"));

	Damage = 50.f;
	PelletsPerShot = 1;
	HeadshotMultiplier = 2.f;
	LimbMultiplier = 1.f;
	RangeMeters = 60.f;

	FRichCurve* Falloff = DamageFalloffCurve.GetRichCurve();
	Falloff->Reset();
	Falloff->AddKey(0.f, 1.f);
	Falloff->AddKey(20.f, 1.f);
	Falloff->AddKey(40.f, 0.6f);
	Falloff->AddKey(60.f, 0.6f);

	RoundsPerMinute = 150.f;
	bFullAuto = false;
	FireInputBuffer = 0.1f;

	EquipTime = 0.4f;
	bUsableWhileCarrying = true;
	MaxUsableWetness = ENFWetness::Waist;

	MagazineSize = 6;
	bInfiniteReserve = true;
	ReloadTime = 2.2f;

	HipSpread = 1.2f;
	AdsSpread = 0.1f;
	MoveSpread = 0.8f;
	MoveSpreadReferenceSpeed = 500.f;
	AirborneSpread = 3.f;
	SpreadPerShot = 0.6f;
	MaxBloom = 2.f;
	SpreadRecoveryPerSecond = 6.f;
	AdsSpreadModifier = 0.3f;

	RecoilPattern = {
		FVector2D(0.0f, 2.4f),
		FVector2D(0.2f, 2.2f),
		FVector2D(-0.2f, 2.2f),
		FVector2D(0.3f, 2.0f),
		FVector2D(-0.3f, 2.0f),
		FVector2D(0.0f, 2.0f)
	};
	RecoilLoopStartIndex = 3;
	RecoilRandomness = 0.15f;
	AdsRecoilMultiplier = 0.7f;
	RecoilKickSpeed = 30.f;
	RecoilRecoveryDelay = 0.1f;
	RecoilRecoverySpeed = 14.f;
	RecoilResetTime = 0.45f;

	AdsFOV = 70.f;
	AdsTime = 0.18f;
}

UNFGafferData::UNFGafferData()
{
	DisplayName = FText::FromString(TEXT("Gaffer"));

	// 15 per pellet x 8 pellets = 120.
	Damage = 15.f;
	PelletsPerShot = 8;
	HeadshotMultiplier = 1.2f;
	LimbMultiplier = 1.f;
	RangeMeters = 25.f;

	FRichCurve* Falloff = DamageFalloffCurve.GetRichCurve();
	Falloff->Reset();
	Falloff->AddKey(0.f, 1.f);
	Falloff->AddKey(7.f, 1.f);
	Falloff->AddKey(18.f, 0.3f);
	Falloff->AddKey(25.f, 0.3f);

	RoundsPerMinute = 70.f;
	bFullAuto = false;
	FireInputBuffer = 0.15f;

	EquipTime = 0.4f;
	bUsableWhileCarrying = false;
	// Two-handed: down from waist depth.
	MaxUsableWetness = ENFWetness::Knee;

	MagazineSize = 6;
	bInfiniteReserve = false;
	StartingReserveAmmo = 30;
	ReloadTime = 2.8f;

	// The pellet cone: every pellet is independent inside it.
	HipSpread = 3.5f;
	AdsSpread = 2.8f;
	MoveSpread = 0.5f;
	MoveSpreadReferenceSpeed = 500.f;
	AirborneSpread = 1.5f;
	SpreadPerShot = 0.f;
	MaxBloom = 0.f;
	// The spec gives no ADS modifier for the Gaffer's move/airborne spread; 1 = unchanged.
	AdsSpreadModifier = 1.f;

	RecoilPattern = { FVector2D(0.0f, 5.0f) };
	RecoilLoopStartIndex = 0;
	RecoilRandomness = 0.6f;
	AdsRecoilMultiplier = 0.8f;
	RecoilKickSpeed = 40.f;
	RecoilRecoveryDelay = 0.15f;
	RecoilRecoverySpeed = 10.f;
	RecoilResetTime = 0.5f;

	AdsFOV = 80.f;
	AdsTime = 0.22f;
}
