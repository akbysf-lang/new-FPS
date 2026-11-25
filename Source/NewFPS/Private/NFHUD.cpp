#include "NFHUD.h"
#include "NFHealthComponent.h"
#include "NFWeaponComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

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
