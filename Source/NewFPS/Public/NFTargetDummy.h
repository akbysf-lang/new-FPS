#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NFTargetDummy.generated.h"

class UMaterialInstanceDynamic;
class UNFHealthComponent;
class USceneComponent;
class UStaticMeshComponent;

/**
 * Grey-box target: a body cylinder and a head sphere built from engine basic shapes.
 * Flashes and pulses when hit, disappears when killed and comes back after RespawnDelay.
 * Drag it into a level; no assets required.
 */
UCLASS()
class NEWFPS_API ANFTargetDummy : public AActor
{
	GENERATED_BODY()

public:
	ANFTargetDummy();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleHealthChanged(UNFHealthComponent* InHealthComponent, float NewHealth, float Delta, AController* InstigatedBy);

	UFUNCTION()
	void HandleDeath(UNFHealthComponent* InHealthComponent, AController* Killer);

	UFUNCTION()
	void HandleRevived(UNFHealthComponent* InHealthComponent);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	/** Tagged "NF.Head": hits on this component use the weapon's headshot multiplier. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HeadMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNFHealthComponent> HealthComponent;

	/** Seconds between death and respawn. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target", meta = (ClampMin = "0.1"))
	float RespawnDelay = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target", meta = (ClampMin = "0.02"))
	float HitReactDuration = 0.15f;

	/** Extra uniform scale at the moment of the hit (0.08 = 8%). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target", meta = (ClampMin = "0.0"))
	float HitReactScalePunch = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	FLinearColor BaseColor = FLinearColor(0.35f, 0.4f, 0.45f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target")
	FLinearColor HitColor = FLinearColor(1.f, 0.2f, 0.1f);

private:
	void Respawn();
	void ApplyHitReact(float Alpha);

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> HeadMaterial;

	float HitReactTimeRemaining = 0.f;
	FTimerHandle RespawnTimerHandle;
};
