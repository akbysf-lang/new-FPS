#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NFSupplyPoint.generated.h"

class ANFCharacter;
class UMaterialInstanceDynamic;
class USceneComponent;
class UStaticMeshComponent;

/** The two settings of a supply point (prototype-spec.md 6.1 and 4.2). */
UENUM(BlueprintType)
enum class ENFSupplyType : uint8
{
	/** First aid box: hold to heal to full. One use per player in the session, shared by the team. */
	FirstAid,
	/** Ammo crate: hold to refill the reserve of every weapon with a limited reserve (the Gaffer). Never runs out. */
	Ammo
};

/**
 * First aid box or ammo crate, one class with two settings (prototype-spec.md 6.1, 4.2, 7.6: placed in the open).
 * Put it on the floor (the actor origin is the floor height) and pick Supply Type on the instance.
 *
 * Used through the player's UNFInteractionComponent: hold Interact within the rules' InteractRange. Server
 * authoritative; the server runs the hold and applies the effect. Only the number of uses spent is replicated;
 * what is left is derived from it and the player count on every machine (GetRemainingUsesFor).
 * The numbers (hold times, health, uses per player) are in UNFRulesData -> Supply.
 */
UCLASS()
class NEWFPS_API ANFSupplyPoint : public AActor
{
	GENERATED_BODY()

public:
	ANFSupplyPoint();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Supply")
	ENFSupplyType GetSupplyType() const { return SupplyType; }

	/** Uses left, read with the rules of User (class defaults when null). -1 = unlimited (ammo). Any machine. */
	UFUNCTION(BlueprintPure, Category = "Supply")
	int32 GetRemainingUsesFor(const ANFCharacter* User) const;

	/** Would holding Interact here do anything for User right now? Any machine (the HUD prompt asks too). */
	UFUNCTION(BlueprintPure, Category = "Supply")
	bool CanBeUsedBy(const ANFCharacter* User) const;

	/** How long User has to hold Interact here. */
	float GetHoldSecondsFor(const ANFCharacter* User) const;

	/** Server only: the hold finished. Heals or refills User. False when nothing was done. */
	bool AuthUse(ANFCharacter* User);

protected:
	virtual void BeginPlay() override;

	/** Set per instance in the level. Not replicated: it is part of the placed actor on every machine. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Supply")
	ENFSupplyType SupplyType = ENFSupplyType::FirstAid;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/** Grey-box placeholder (60 x 40 x 30 cm, tinted by type). Replace the mesh in a Blueprint when the model exists. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BoxMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Supply|Presentation")
	FLinearColor FirstAidColor = FLinearColor(0.9f, 0.9f, 0.9f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Supply|Presentation")
	FLinearColor AmmoColor = FLinearColor(0.22f, 0.28f, 0.12f);

private:
	/** First aid uses spent so far. Server writes, everyone reads. */
	UPROPERTY(Replicated)
	int32 UsesConsumed = 0;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> BoxMaterial;
};
