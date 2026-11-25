#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NFInteractionComponent.generated.h"

class ANFCharacter;
class ANFStrongbox;
class ANFSupplyPoint;

/** What a held Interact button is doing right now. */
UENUM(BlueprintType)
enum class ENFInteraction : uint8
{
	None,
	/** Lifting a strongbox. */
	PickUp,
	/** Getting a downed teammate back up. */
	Revive,
	/** Using a first aid box or an ammo crate (ANFSupplyPoint). */
	Supply
};

/**
 * Hold-to-interact and strongbox carrying for a player character (prototype-spec.md 2.3, 6.2, 6.4).
 *
 * Server authoritative: the client only reports "Interact is held / released". The server picks the target
 * (a downed teammate first, then the nearer of a free strongbox and a usable supply point), runs the hold timer,
 * and completes or cancels it.
 * The carried strongbox is replicated to everyone; the running interaction only to the owner (for the HUD).
 */
UCLASS(ClassGroup = (NewFPS), meta = (BlueprintSpawnableComponent))
class NEWFPS_API UNFInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNFInteractionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ---- Input (call on the owning client) ----

	/** Interact pressed: drops the strongbox when carrying one, otherwise starts a hold interaction. */
	void BeginInteract();

	/** Interact released: a hold that was not finished is lost. */
	void EndInteract();

	/** Put the strongbox down (also bound to the weapon switch inputs). */
	void RequestDrop();

	// ---- State ----

	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsCarrying() const { return CarriedBox != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	ANFStrongbox* GetCarriedBox() const { return CarriedBox; }

	/** Owning client and server only. */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	ENFInteraction GetActiveInteraction() const { return ActiveInteraction; }

	/** 0..1 through the current hold (local estimate on the owning client). 0 when nothing is running. */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetInteractionProgress() const;

	/**
	 * Nearest supply point within the interact range of the owner, or null. Any machine: the server picks its
	 * target with it (bUsableOnly), the HUD of the owning client draws its prompt from it.
	 */
	ANFSupplyPoint* FindSupplyPointInRange(bool bUsableOnly) const;

	// ---- Server only ----

	/** Drop the strongbox at the owner's feet (never thrown). Delivers it when that is inside a winch net. */
	void AuthDropCarried();

	/** Abort a running hold (the owner went down, etc.). */
	void AuthCancelInteraction();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(Server, Reliable)
	void ServerSetInteracting(bool bInteracting);

	UFUNCTION(Server, Reliable)
	void ServerDropCarried();

	UFUNCTION()
	void OnRep_CarriedBox();

	UFUNCTION()
	void OnRep_ActiveInteraction();

private:
	ANFCharacter* GetNFOwner() const;
	float GetWorldTime() const;
	bool IsWithinRange(const AActor* Other, float Range) const;

	void AuthBeginInteraction();
	void AuthSetInteraction(ENFInteraction NewInteraction, AActor* NewTarget, float Duration);
	bool AuthIsInteractionStillValid() const;
	void AuthCompleteInteraction();

	/** Mirror the carry state into the owner (movement, weapon, proxy meshes). Runs on every machine. */
	void ApplyCarryState();

	UPROPERTY(ReplicatedUsing = OnRep_CarriedBox)
	TObjectPtr<ANFStrongbox> CarriedBox;

	UPROPERTY(ReplicatedUsing = OnRep_ActiveInteraction)
	ENFInteraction ActiveInteraction = ENFInteraction::None;

	UPROPERTY(Replicated)
	float ActiveInteractionDuration = 0.f;

	/** Server: what the running hold is aimed at. */
	UPROPERTY(Transient)
	TObjectPtr<AActor> InteractionTarget;

	/** World time (of this machine) the running hold started. */
	float InteractionStartTime = 0.f;
};
