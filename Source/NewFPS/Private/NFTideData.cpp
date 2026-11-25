#include "NFTideData.h"
#include "Curves/CurveFloat.h"

UNFTideData::UNFTideData()
{
	// prototype-spec.md 3.2: one 12:00 cycle, piecewise linear. Weights are seconds (they sum to CycleDurationSeconds),
	// levels are world Z / HighWaterZ. Change the numbers in an NFTideData asset, not here.
	const float High = 830.f;
	auto AddPhase = [this, High](ENFTidePhase Phase, float Seconds, float StartZ, float EndZ)
	{
		FNFTidePhaseEntry Entry;
		Entry.Phase = Phase;
		Entry.DurationWeight = Seconds;
		Entry.StartLevel = StartZ / High;
		Entry.EndLevel = EndZ / High;
		Phases.Add(Entry);
	};

	AddPhase(ENFTidePhase::Ebb, 90.f, 830.f, 200.f);        // E1  0:00-1:30
	AddPhase(ENFTidePhase::Ebb, 30.f, 200.f, 150.f);        // E2  1:30-2:00
	AddPhase(ENFTidePhase::LowSlack, 120.f, 150.f, 0.f);    // SL1 2:00-4:00 (still draining)
	AddPhase(ENFTidePhase::LowSlack, 60.f, 0.f, 0.f);       // SL2 4:00-5:00
	AddPhase(ENFTidePhase::Flood, 90.f, 0.f, 300.f);        // F1  5:00-6:30 beach goes
	AddPhase(ENFTidePhase::Flood, 30.f, 300.f, 300.f);      // F2  6:30-7:00 half tide pause
	AddPhase(ENFTidePhase::Flood, 105.f, 300.f, 700.f);     // F3  7:00-8:45 half level goes
	AddPhase(ENFTidePhase::Flood, 15.f, 700.f, 700.f);      // F4  8:45-9:00
	AddPhase(ENFTidePhase::Flood, 60.f, 700.f, 830.f);      // F5  9:00-10:00 terrace gets wet
	AddPhase(ENFTidePhase::HighSlack, 120.f, 830.f, 830.f); // SH  10:00-12:00

	auto AddWarning = [this](const TCHAR* Id, ENFTidePhase Upcoming, float SecondsBefore)
	{
		FNFTideWarning Warning;
		Warning.WarningId = FName(Id);
		Warning.UpcomingPhase = Upcoming;
		Warning.SecondsBefore = SecondsBefore;
		Warnings.Add(Warning);
	};

	// Spec 3.4: T-60 (clock flashes), T-30 (one bell), T-10 (three bells) before the tide turns.
	AddWarning(TEXT("FloodT60"), ENFTidePhase::Flood, 60.f);
	AddWarning(TEXT("FloodT30"), ENFTidePhase::Flood, 30.f);
	AddWarning(TEXT("FloodT10"), ENFTidePhase::Flood, 10.f);
	AddWarning(TEXT("HighWaterSoon"), ENFTidePhase::HighSlack, 20.f);
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
	// Smoothstep hides the jerk where an entry starts or ends; the spec's table is linear.
	return bSmoothPhaseEase ? Alpha * Alpha * (3.f - 2.f * Alpha) : Alpha;
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

	// Time until the label changes: walk forward over entries that carry the same label.
	double LabelRemaining = PhaseLength - PhaseElapsed;
	ENFTidePhase NextLabel = Entry.Phase;
	int32 WalkIndex = FoundIndex;
	for (int32 Step = 0; Step < NumPhases; ++Step)
	{
		int32 Following = WalkIndex + 1;
		if (Following >= NumPhases)
		{
			if (!bLoop)
			{
				break;
			}
			Following = 0;
		}
		if (Phases[Following].Phase != Entry.Phase)
		{
			NextLabel = Phases[Following].Phase;
			break;
		}
		LabelRemaining += CycleDuration * FMath::Max(Phases[Following].DurationWeight, 0.f) / TotalWeight;
		WalkIndex = Following;
	}

	Sample.bValid = true;
	Sample.PhaseIndex = FoundIndex;
	Sample.Phase = Entry.Phase;
	Sample.NextPhase = Phases[NextIndex].Phase;
	Sample.PhaseDuration = static_cast<float>(PhaseLength);
	Sample.PhaseElapsed = static_cast<float>(PhaseElapsed);
	Sample.PhaseRemaining = static_cast<float>(PhaseLength - PhaseElapsed);
	Sample.LabelRemaining = static_cast<float>(LabelRemaining);
	Sample.NextLabel = NextLabel;
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

	if (DepthCm <= WaistDepth)
	{
		const float WaistRange = FMath::Max(WaistDepth - KneeDepth, 1.f);
		const float Alpha = FMath::Clamp((DepthCm - KneeDepth) / WaistRange, 0.f, 1.f);
		return FMath::Lerp(KneeSpeedMultiplier, WaistSpeedMultiplier, Alpha);
	}

	// Waist -> chest keeps ramping (still continuous: see the tide ADR); flat from ChestDepth down.
	const float ChestRange = FMath::Max(ChestDepth - WaistDepth, 1.f);
	const float Alpha = FMath::Clamp((DepthCm - WaistDepth) / ChestRange, 0.f, 1.f);
	return FMath::Lerp(WaistSpeedMultiplier, ChestSpeedMultiplier, Alpha);
}

float UNFTideData::GetJumpMultiplierForDepth(float DepthCm) const
{
	if (DepthCm >= WaistDepth)
	{
		return 0.f;
	}
	const float BlendStart = KneeDepth - FMath::Max(WadeBlendDepth, 1.f);
	const float Alpha = FMath::Clamp((DepthCm - BlendStart) / (KneeDepth - BlendStart), 0.f, 1.f);
	return FMath::Lerp(1.f, KneeJumpMultiplier, Alpha);
}

float UNFTideData::GetFirearmDepthLimit(ENFWetness MaxUsableWetness) const
{
	float Limit = ChestDepth;
	switch (MaxUsableWetness)
	{
	case ENFWetness::Dry:
		Limit = KneeDepth;
		break;
	case ENFWetness::Knee:
		Limit = WaistDepth;
		break;
	default:
		break;
	}
	return FMath::Min(Limit, ChestDepth);
}
