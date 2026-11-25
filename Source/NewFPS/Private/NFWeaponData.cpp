#include "NFWeaponData.h"

UNFWeaponData::UNFWeaponData()
{
	DisplayName = FText::FromString(TEXT("Prototype Rifle"));

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
