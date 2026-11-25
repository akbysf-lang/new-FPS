#include "NFTideData.h"
#include "Curves/CurveFloat.h"

UNFTideData::UNFTideData()
{
	// Placeholder table: 30 s low slack, 90 s flood, 30 s high slack, 90 s ebb (weights 1/3/1/3 of 240 s).
	FNFTidePhaseEntry LowSlack;
	LowSlack.Phase = ENFTidePhase::LowSlack;
	LowSlack.DurationWeight = 1.f;
	LowSlack.StartLevel = 0.f;
	LowSlack.EndLevel = 0.f;

	FNFTidePhaseEntry Flood;
	Flood.Phase = ENFTidePhase::Flood;
	Flood.DurationWeight = 3.f;
	Flood.StartLevel = 0.f;
	Flood.EndLevel = 1.f;

	FNFTidePhaseEntry HighSlack;
	HighSlack.Phase = ENFTidePhase::HighSlack;
	HighSlack.DurationWeight = 1.f;
	HighSlack.StartLevel = 1.f;
	HighSlack.EndLevel = 1.f;

	FNFTidePhaseEntry Ebb;
	Ebb.Phase = ENFTidePhase::Ebb;
	Ebb.DurationWeight = 3.f;
	Ebb.StartLevel = 1.f;
	Ebb.EndLevel = 0.f;

	Phases.Add(LowSlack);
	Phases.Add(Flood);
	Phases.Add(HighSlack);
	Phases.Add(Ebb);

	FNFTideWarning FloodSoon;
	FloodSoon.WarningId = TEXT("FloodSoon");
	FloodSoon.UpcomingPhase = ENFTidePhase::Flood;
	FloodSoon.SecondsBefore = 20.f;

	FNFTideWarning FloodNow;
	FloodNow.WarningId = TEXT("FloodImminent");
	FloodNow.UpcomingPhase = ENFTidePhase::Flood;
	FloodNow.SecondsBefore = 5.f;

	FNFTideWarning HighWaterSoon;
	HighWaterSoon.WarningId = TEXT("HighWaterSoon");
	HighWaterSoon.UpcomingPhase = ENFTidePhase::HighSlack;
	HighWaterSoon.SecondsBefore = 20.f;

	Warnings.Add(FloodSoon);
	Warnings.Add(FloodNow);
	Warnings.Add(HighWaterSoon);
}

FPrimaryAssetId UNFTideData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType(TEXT("NFTide")), GetFName());
}

float UNFTideData::GetTotalWeight() const
{
	float Total = 0.f;
	for (const FNFTidePhaseEntry& Entry : Phases)
	{
		Total += FMath::Max(Entry.DurationWeight, 0.f);
	}
	return Total;
}

float UNFTideData::GetPhaseDuration(int32 PhaseIndex) const
{
	const float TotalWeight = GetTotalWeight();
	if (!Phases.IsValidIndex(PhaseIndex) || TotalWeight <= KINDA_SMALL_NUMBER)
	{
		return 0.f;
	}
	return FMath::Max(CycleDurationSeconds, 1.f) * FMath::Max(Phases[PhaseIndex].DurationWeight, 0.f) / TotalWeight;
}

float UNFTideData::EaseLevel(float Alpha) const
{
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);
	if (HeightEaseCurve)
	{
		return FMath::Clamp(HeightEaseCurve->GetFloatValue(Alpha), 0.f, 1.f);
	}
	// Smoothstep: no visible jerk where a phase starts or ends.
	return Alpha * Alpha * (3.f - 2.f * Alpha);
}

FNFTideSample UNFTideData::Evaluate(double ElapsedSeconds) const
{
	FNFTideSample Sample;
	Sample.WaterZ = LowWaterZ;

	const int32 NumPhases = Phases.Num();
	const float TotalWeight = GetTotalWeight();
	if (NumPhases == 0 || TotalWeight <= KINDA_SMALL_NUMBER)
	{
		return Sample;
	}

	const double CycleDuration = FMath::Max(CycleDurationSeconds, 1.f);
	ElapsedSeconds = FMath::Max(ElapsedSeconds, 0.0);

	// Time inside the current cycle. Kept in double: the server clock runs for hours.
	double CycleTime = 0.0;
	bool bFinished = false;
	if (bLoop)
	{
		const double CycleCount = FMath::FloorToDouble(ElapsedSeconds / CycleDuration);
		Sample.CycleIndex = static_cast<int32>(CycleCount);
		CycleTime = ElapsedSeconds - CycleCount * CycleDuration;
	}
	else if (ElapsedSeconds >= CycleDuration)
	{
		CycleTime = CycleDuration;
		bFinished = true;
	}
	else
	{
		CycleTime = ElapsedSeconds;
	}

	int32 FoundIndex = NumPhases - 1;
	double PhaseStart = 0.0;
	double PhaseLength = 0.0;
	double Cursor = 0.0;
	for (int32 Index = 0; Index < NumPhases; ++Index)
	{
		const double Length = CycleDuration * FMath::Max(Phases[Index].DurationWeight, 0.f) / TotalWeight;
		const bool bLast = (Index == NumPhases - 1);
		if (bLast || (!bFinished && Length > 0.0 && CycleTime < Cursor + Length))
		{
			FoundIndex = Index;
			PhaseStart = Cursor;
			PhaseLength = Length;
			break;
		}
		Cursor += Length;
	}

	const FNFTidePhaseEntry& Entry = Phases[FoundIndex];
	const double PhaseElapsed = FMath::Clamp(CycleTime - PhaseStart, 0.0, PhaseLength);
	const float Alpha = PhaseLength > KINDA_SMALL_NUMBER ? static_cast<float>(PhaseElapsed / PhaseLength) : 1.f;

	const int32 NextIndex = (FoundIndex + 1 < NumPhases) ? FoundIndex + 1 : (bLoop ? 0 : FoundIndex);

	Sample.bValid = true;
	Sample.PhaseIndex = FoundIndex;
	Sample.Phase = Entry.Phase;
	Sample.NextPhase = Phases[NextIndex].Phase;
	Sample.PhaseDuration = static_cast<float>(PhaseLength);
	Sample.PhaseElapsed = static_cast<float>(PhaseElapsed);
	Sample.PhaseRemaining = static_cast<float>(PhaseLength - PhaseElapsed);
	Sample.Level = FMath::Lerp(Entry.StartLevel, Entry.EndLevel, EaseLevel(Alpha));
	Sample.WaterZ = FMath::Lerp(LowWaterZ, HighWaterZ, Sample.Level);
	return Sample;
}

ENFWetness UNFTideData::ClassifyDepth(float DepthCm) const
{
	if (DepthCm >= OverheadDepth)
	{
		return ENFWetness::Overhead;
	}
	if (DepthCm >= WaistDepth)
	{
		return ENFWetness::Waist;
	}
	if (DepthCm >= KneeDepth)
	{
		return ENFWetness::Knee;
	}
	return ENFWetness::Dry;
}

ENFWetness UNFTideData::GetWetnessForDepth(float DepthCm, ENFWetness CurrentWetness) const
{
	// Getting wetter is immediate; getting drier needs the water to fall WetnessHysteresis below the threshold.
	const ENFWetness Rising = ClassifyDepth(DepthCm);
	if (Rising >= CurrentWetness)
	{
		return Rising;
	}
	const ENFWetness Falling = ClassifyDepth(DepthCm + WetnessHysteresis);
	return Falling < CurrentWetness ? Falling : CurrentWetness;
}

float UNFTideData::GetSpeedMultiplierForDepth(float DepthCm) const
{
	const float BlendStart = KneeDepth - FMath::Max(WadeBlendDepth, 1.f);
	if (DepthCm <= BlendStart)
	{
		return 1.f;
	}
	if (DepthCm <= KneeDepth)
	{
		const float Alpha = (DepthCm - BlendStart) / (KneeDepth - BlendStart);
		return FMath::Lerp(1.f, KneeSpeedMultiplier, Alpha);
	}

	const float WaistRange = FMath::Max(WaistDepth - KneeDepth, 1.f);
	const float Alpha = FMath::Clamp((DepthCm - KneeDepth) / WaistRange, 0.f, 1.f);
	return FMath::Lerp(KneeSpeedMultiplier, WaistSpeedMultiplier, Alpha);
}
