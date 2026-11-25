#include "NFHUD.h"
#include "NFHealthComponent.h"
#include "NFTideManager.h"
#include "NFWeaponComponent.h"
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
		const FString AmmoText = Weapon->IsReloading()
			? FString(TEXT("RELOADING"))
			: (Reserve < 0
				? FString::Printf(TEXT("%d / INF"), Weapon->GetAmmoInMagazine())
				: FString::Printf(TEXT("%d / %d"), Weapon->GetAmmoInMagazine(), Reserve));
		DrawText(AmmoText, FLinearColor::White, Canvas->ClipX - 220.f, Canvas->ClipY - 80.f, nullptr, 2.f);
	}

	if (Health)
	{
		const FString HealthText = Health->IsDead()
			? FString(TEXT("DEAD"))
			: FString::Printf(TEXT("HP %d"), FMath::CeilToInt(Health->GetHealth()));
		DrawText(HealthText, Health->IsDead() ? FLinearColor::Red : FLinearColor::White, 60.f, Canvas->ClipY - 80.f, nullptr, 2.f);
	}

	DrawTide(Pawn);

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
		const int32 SecondsLeft = FMath::CeilToInt(Sample.PhaseRemaining);
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
			WetnessText = TEXT("TOO DEEP TO SHOOT");
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
