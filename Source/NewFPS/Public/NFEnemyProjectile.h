#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NFEnemyProjectile.generated.h"

class UPrimitiveComponent;
class UProjectileMovementComponent;
class USphereComponent;
class UStaticMeshComponent;
struct FHitResult;

/**
 * The Attendant's slow punch projectile (prototype-spec.md 5.2): flies straight, no gravity, damages the first
 * player it touches and stops at the world. Spawned and resolved on the server; clients see the replicated actor.
 */
UCLASS()
class NEWFPS_API ANFEnemyProjectile : public AActor
{
	GENERATED_BODY()

public:
	ANFEnemyProjectile();

	/** Server only, right after spawning: set the damage and (optionally) another speed than the class default. */
	void InitProjectile(float InDamage, float InSpeed);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleStop(const FHitResult& ImpactResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> Collision;

	/** Grey-box visual (engine sphere). Replace or clear in a Blueprint when the VFX exists. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Visual;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> Movement;

private:
	float Damage = 10.f;
	bool bSpent = false;
};
