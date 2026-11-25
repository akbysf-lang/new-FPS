#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "NFTypes.h"
#include "NFHUD.generated.h"

class UNFWeaponComponent;

/** Minimal Canvas HUD: spread crosshair, ammo, health and hit marker. No UMG assets required. */
UCLASS()
class NEWFPS_API ANFHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "NewFPS|Crosshair")
	FLinearColor CrosshairColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category = "NewFPS|Crosshair", meta = (ClampMin = "1.0"))
	float CrosshairLineLength = 8.f;

	UPROPERTY(EditDefaultsOnly, Category = "NewFPS|Crosshair", meta = (ClampMin = "0.0"))
	float CrosshairMinGap = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "NewFPS|Crosshair", meta = (ClampMin = "1.0"))
	float CrosshairThickness = 2.f;

	UPROPERTY(EditDefaultsOnly, Category = "NewFPS|HitMarker", meta = (ClampMin = "0.05"))
	float HitMarkerDuration = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = "NewFPS|HitMarker")
	FLinearColor HitMarkerColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category = "NewFPS|HitMarker")
	FLinearColor HeadshotMarkerColor = FLinearColor(1.f, 0.8f, 0.1f);

	UPROPERTY(EditDefaultsOnly, Category = "NewFPS|HitMarker")
	FLinearColor KillMarkerColor = FLinearColor(1.f, 0.15f, 0.1f);

private:
	UFUNCTION()
	void HandleHitConfirmed(AActor* HitActor, float Damage, ENFHitZone Zone, bool bKilled);

	void BindToWeapon(UNFWeaponComponent* Weapon);

	TWeakObjectPtr<UNFWeaponComponent> BoundWeapon;
	float HitMarkerEndTime = -1.f;
	FLinearColor ActiveHitMarkerColor = FLinearColor::White;
	float LastHitDamage = 0.f;
};
