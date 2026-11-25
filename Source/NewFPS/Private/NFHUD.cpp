#include "NFHUD.h"
#include "NFCharacter.h"
#include "NFGameState.h"
#include "NFHealthComponent.h"
#include "NFInteractionComponent.h"
#include "NFSupplyPoint.h"
#include "NFTideData.h"
#include "NFTideManager.h"
#include "NFWeaponComponent.h"
#include "NFWeaponData.h"
#include "NFWetnessComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarNFTideDebug(
	TEXT("nf.Tide.Debug"),
	1,
	TEXT("Show water height, tide clock and local water depth under the tide indicator. 0 = off."));

void ANFHUD::BindToWeapon(UNFWeaponComponent* Weapon)
{
	if (UNFWeaponComponent* OldWeapon = BoundWeapon.Get())
	{
		OldWeapon->OnHitConfirmed.RemoveDynamic(this, &ANFHUD::HandleHitConfirmed);
	}

	BoundWeapon = Weapon;

	if (Weapon)
	{
		Weapon->OnHitConfirmed.AddUniqueDynamic(this, &ANFHUD::HandleHitConfirmed);
	}
}

void ANFHUD::HandleHitConfirmed(AActor* HitActor, float Damage, ENFHitZone Zone, bool bKilled)
{
	HitMarkerEndTime = GetWorld()->GetTimeSeconds() + HitMarkerDuration * (bKilled ? 2.f : 1.f);
	ActiveHitMarkerColor = bKilled ? KillMarkerColor : (Zone == ENFHitZone::Head ? HeadshotMarkerColor : HitMarkerColor);
	LastHitDamage = Damage;
}

void ANFHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	const APawn* Pawn = GetOwningPawn();
	UNFWeaponComponent* Weapon = Pawn ? Pawn->FindComponentByClass<UNFWeaponComponent>() : nullptr;
	if (Weapon != BoundWeapon.Get())
	{
		BindToWeapon(Weapon);
	}

	const float CenterX = Canvas->ClipX * 0.5f;
	const float CenterY = Canvas->ClipY * 0.5f;
	const float Now = GetWorld()->GetTimeSeconds();

	const UNFHealthComponent* Health = UNFHealthComponent::FindHealthComponent(Pawn);
	const bool bAlive = Pawn && (!Health || !Health->IsDead());
	const ANFCharacter* ViewedCharacter = Cast<ANFCharacter>(Pawn);

	if (Weapon && bAlive)
	{
		// Crosshair gap follows the real spread cone projected on screen.
		float FOV = 90.f;
		if (PlayerOwner && PlayerOwner->PlayerCameraManager)
		{
			FOV = PlayerOwner->PlayerCameraManager->GetFOVAngle();
		}
		const float HalfFOVTan = FMath::Tan(FMath::DegreesToRadians(FMath::Clamp(FOV, 10.f, 170.f) * 0.5f));
		const float SpreadTan = FMath::Tan(FMath::DegreesToRadians(Weapon->GetCurrentSpreadDegrees()));
		const float Gap = CrosshairMinGap + (SpreadTan / HalfFOVTan) * CenterX;

		// Fade the crosshair out while aiming down sights.
		FLinearColor Color = CrosshairColor;
		Color.A *= 1.f - Weapon->GetAdsAlpha();

		if (Color.A > 0.02f)
		{
			DrawLine(CenterX - Gap - CrosshairLineLength, CenterY, CenterX - Gap, CenterY, Color, CrosshairThickness);
			DrawLine(CenterX + Gap, CenterY, CenterX + Gap + CrosshairLineLength, CenterY, Color, CrosshairThickness);
			DrawLine(CenterX, CenterY - Gap - CrosshairLineLength, CenterX, CenterY - Gap, Color, CrosshairThickness);
			DrawLine(CenterX, CenterY + Gap, CenterX, CenterY + Gap + CrosshairLineLength, Color, CrosshairThickness);
		}
		DrawRect(CrosshairColor, CenterX - 1.f, CenterY - 1.f, 2.f, 2.f);

		const int32 Reserve = Weapon->GetReserveAmmo();
		FString AmmoText;
		if (Weapon->IsReloading())
		{
			AmmoText = TEXT("RELOADING");
		}
		else if (Weapon->IsSwitching())
		{
			AmmoText = TEXT("...");
		}
		else if (Reserve < 0)
		{
			AmmoText = FString::Printf(TEXT("%d / INF"), Weapon->GetAmmoInMagazine());
		}
		else
		{
			AmmoText = FString::Printf(TEXT("%d / %d"), Weapon->GetAmmoInMagazine(), Reserve);
		}

		// A firearm the water or the strongbox has taken out of play is shown greyed.
		const FLinearColor WeaponColor = Weapon->IsFirearmUnusable() ? FLinearColor(0.5f, 0.5f, 0.5f) : FLinearColor::White;
		DrawText(Weapon->GetData()->DisplayName.ToString().ToUpper(), WeaponColor, Canvas->ClipX - 220.f, Canvas->ClipY - 112.f, nullptr, 1.5f);
		DrawText(AmmoText, WeaponColor, Canvas->ClipX - 220.f, Canvas->ClipY - 80.f, nullptr, 2.f);
	}

	if (Health)
	{
		FString HealthText = FString::Printf(TEXT("HP %d"), FMath::CeilToInt(Health->GetHealth()));
		FLinearColor HealthColor = FLinearColor::White;
		if (ViewedCharacter && ViewedCharacter->GetLifeState() == ENFLifeState::Downed)
		{
			const int32 SecondsLeft = FMath::CeilToInt(ViewedCharacter->GetBleedoutRemaining());
			HealthText = FString::Printf(TEXT("DOWNED  %d:%02d"), SecondsLeft / 60, SecondsLeft % 60);
			HealthColor = FLinearColor::Red;
		}
		else if (ViewedCharacter && ViewedCharacter->GetLifeState() == ENFLifeState::Out)
		{
			HealthText = TEXT("HAULED UP - WAIT FOR THE LINE");
			HealthColor = FLinearColor::Red;
		}
		else if (Health->IsDead())
		{
			HealthText = TEXT("DEAD");
			HealthColor = FLinearColor::Red;
		}
		DrawText(HealthText, HealthColor, 60.f, Canvas->ClipY - 80.f, nullptr, 2.f);
	}

	DrawTide(Pawn);
	DrawMission(ViewedCharacter);

	if (Now < HitMarkerEndTime)
	{
		FLinearColor Color = ActiveHitMarkerColor;
		Color.A = FMath::Clamp((HitMarkerEndTime - Now) / HitMarkerDuration, 0.f, 1.f);

		const float Inner = 6.f;
		const float Outer = 14.f;
		DrawLine(CenterX - Outer, CenterY - Outer, CenterX - Inner, CenterY - Inner, Color, 2.f);
		DrawLine(CenterX + Outer, CenterY - Outer, CenterX + Inner, CenterY - Inner, Color, 2.f);
		DrawLine(CenterX - Outer, CenterY + Outer, CenterX - Inner, CenterY + Inner, Color, 2.f);
		DrawLine(CenterX + Outer, CenterY + Outer, CenterX + Inner, CenterY + Inner, Color, 2.f);

		DrawText(FString::Printf(TEXT("%d"), FMath::RoundToInt(LastHitDamage)), Color, CenterX + 22.f, CenterY - 26.f, nullptr, 1.2f);
	}
}

void ANFHUD::DrawTide(const APawn* ViewedPawn)
{
	const ANFTideManager* TideManager = ANFTideManager::Get(GetWorld());
	if (!TideManager)
	{
		return;
	}

	const FNFTideSample Sample = TideManager->GetCurrentSample();
	const float TextX = Canvas->ClipX * 0.5f - 110.f;

	if (Sample.bValid)
	{
		// Time until the tide turns (the label changes), not until the next table entry.
		const int32 SecondsLeft = FMath::CeilToInt(Sample.LabelRemaining);
		const FString TideText = TideManager->IsTideRunning()
			? FString::Printf(TEXT("%s  %d:%02d"), ANFTideManager::GetPhaseLabel(Sample.Phase), SecondsLeft / 60, SecondsLeft % 60)
			: FString::Printf(TEXT("%s  (HELD)"), ANFTideManager::GetPhaseLabel(Sample.Phase));
		DrawText(TideText, Sample.Phase == ENFTidePhase::Flood ? TideAlertColor : TideTextColor, TextX, 40.f, nullptr, 2.f);
	}

	const UNFWetnessComponent* Wetness = ViewedPawn ? ViewedPawn->FindComponentByClass<UNFWetnessComponent>() : nullptr;
	if (Wetness)
	{
		const TCHAR* WetnessText = nullptr;
		switch (Wetness->GetWetness())
		{
		case ENFWetness::Knee:
			WetnessText = TEXT("WADING");
			break;
		case ENFWetness::Waist:
			// Spec 3.3: the firearm lock is the chest depth (135 cm), not the Waist state (100 cm).
			WetnessText = Wetness->GetWaterDepth() >= Wetness->GetTideData()->ChestDepth
				? TEXT("TOO DEEP TO SHOOT")
				: TEXT("WAIST DEEP - PURSER FROM THE HIP ONLY");
			break;
		case ENFWetness::Overhead:
			WetnessText = TEXT("UNDER WATER");
			break;
		default:
			break;
		}
		if (WetnessText)
		{
			DrawText(WetnessText, Wetness->GetWetness() == ENFWetness::Overhead ? FLinearColor::Red : TideAlertColor, TextX, 76.f, nullptr, 1.5f);
		}
	}

	// Compare this line between two clients to verify that the water height matches.
	if (CVarNFTideDebug.GetValueOnGameThread() > 0)
	{
		const FString DebugText = FString::Printf(TEXT("water Z %.1f  level %.3f  tide t %.2f  depth %.0f"),
			TideManager->GetGlobalWaterZ(), Sample.Level, TideManager->GetElapsedSeconds(), Wetness ? Wetness->GetWaterDepth() : 0.f);
		DrawText(DebugText, FLinearColor::White, TextX, 104.f, nullptr, 1.f);
	}
}

void ANFHUD::DrawMission(const ANFCharacter* ViewedCharacter)
{
	const UWorld* World = GetWorld();
	const ANFGameState* NFGameState = World ? World->GetGameState<ANFGameState>() : nullptr;
	const float CenterX = Canvas->ClipX * 0.5f;

	if (NFGameState)
	{
		// Strongbox counter (spec 1.2: minimal HUD).
		const FString QuotaText = FString::Printf(TEXT("STRONGBOXES  %d / %d"), NFGameState->GetDeliveredStrongboxes(), NFGameState->GetQuota());
		DrawText(QuotaText, NFGameState->IsQuotaMet() ? FLinearColor::Green : FLinearColor::White, 60.f, 40.f, nullptr, 1.5f);

		if (NFGameState->GetMissionResult() == ENFMissionResult::Success)
		{
			DrawText(FString(TEXT("THAT'S A SHIFT")), FLinearColor::Green, CenterX - 150.f, Canvas->ClipY * 0.3f, nullptr, 3.f);
		}
		else if (NFGameState->GetMissionResult() == ENFMissionResult::Failed)
		{
			DrawText(FString(TEXT("SHIFT FAILED")), FLinearColor::Red, CenterX - 130.f, Canvas->ClipY * 0.3f, nullptr, 3.f);
		}
	}

	const UNFInteractionComponent* Interaction = ViewedCharacter ? ViewedCharacter->GetInteractionComponent() : nullptr;
	if (!Interaction)
	{
		return;
	}

	const float PromptY = Canvas->ClipY * 0.62f;
	if (Interaction->IsCarrying())
	{
		DrawText(FString(TEXT("CARRYING STRONGBOX - INTERACT OR SWITCH WEAPON TO DROP")), TideAlertColor, CenterX - 260.f, PromptY, nullptr, 1.2f);
	}

	const ENFInteraction Active = Interaction->GetActiveInteraction();

	// Supply point prompt (spec 6.1, 4.2): what is in reach, what it does, and how many uses the team has left.
	// Chest deep the server refuses the hold (same rule as lifting a strongbox), so no prompt there either.
	const ANFSupplyPoint* NearSupply = nullptr;
	if (ViewedCharacter->GetLifeState() == ENFLifeState::Alive && !Interaction->IsCarrying())
	{
		const UNFWetnessComponent* Wetness = ViewedCharacter->GetWetnessComponent();
		if (!Wetness || Wetness->GetWaterDepth() < Wetness->GetTideData()->ChestDepth)
		{
			NearSupply = Interaction->FindSupplyPointInRange(/*bUsableOnly*/ false);
		}
	}
	const bool bFirstAid = NearSupply && NearSupply->GetSupplyType() == ENFSupplyType::FirstAid;
	if (NearSupply && Active == ENFInteraction::None)
	{
		FString PromptText;
		FLinearColor PromptColor = FLinearColor::White;
		if (!bFirstAid)
		{
			PromptText = TEXT("HOLD INTERACT - AMMO");
		}
		else
		{
			const int32 UsesLeft = NearSupply->GetRemainingUsesFor(ViewedCharacter);
			if (UsesLeft <= 0)
			{
				PromptText = TEXT("FIRST AID - EMPTY");
				PromptColor = FLinearColor(0.5f, 0.5f, 0.5f);
			}
			else if (NearSupply->CanBeUsedBy(ViewedCharacter))
			{
				PromptText = FString::Printf(TEXT("HOLD INTERACT - FIRST AID (%d LEFT)"), UsesLeft);
			}
			else
			{
				// Full health: nothing to hold for, but the count is worth knowing.
				PromptText = FString::Printf(TEXT("FIRST AID (%d LEFT)"), UsesLeft);
				PromptColor = FLinearColor(0.5f, 0.5f, 0.5f);
			}
		}
		DrawText(PromptText, PromptColor, CenterX - 150.f, PromptY, nullptr, 1.2f);
	}

	if (Active != ENFInteraction::None)
	{
		const float BarWidth = 200.f;
		const float Progress = Interaction->GetInteractionProgress();
		FString Label = TEXT("LIFTING");
		if (Active == ENFInteraction::Revive)
		{
			Label = TEXT("REVIVING");
		}
		else if (Active == ENFInteraction::Supply)
		{
			Label = bFirstAid ? TEXT("PATCHING UP") : TEXT("RESTOCKING");
		}
		DrawText(Label, FLinearColor::White, CenterX - 40.f, PromptY + 28.f, nullptr, 1.2f);
		DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.5f), CenterX - BarWidth * 0.5f, PromptY + 52.f, BarWidth, 8.f);
		DrawRect(FLinearColor::White, CenterX - BarWidth * 0.5f, PromptY + 52.f, BarWidth * Progress, 8.f);
	}
}
