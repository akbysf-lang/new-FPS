#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NFTypes.h"
#include "NFCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UNFCharacterMovementComponent;
class UNFHealthComponent;
class UNFInteractionComponent;
class UNFMovementData;
class UNFRulesData;
class UNFWeaponComponent;
class UNFWetnessComponent;
class USkeletalMeshComponent;
class UStaticMeshComponent;
struct FInputActionValue;

/**
 * First-person player character: camera, first-person arms/weapon slots, input, and glue between the
 * movement, weapon, health, wetness and interaction components. Gameplay logic lives in the components; the
 * one thing owned here is the co-op life cycle (Alive -> Downed -> Out, prototype-spec.md 6.2).
 */
UCLASS()
class NEWFPS_API ANFCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANFCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaSeconds) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void RecalculateBaseEyeHeight() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PawnClientRestart() override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	UFUNCTION(BlueprintPure, Category = "NewFPS")
	UNFWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }

	UFUNCTION(BlueprintPure, Category = "NewFPS")
	UNFHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintPure, Category = "NewFPS")
	UNFWetnessComponent* GetWetnessComponent() const { return WetnessComponent; }

	UFUNCTION(BlueprintPure, Category = "NewFPS")
	UCameraComponent* GetFirstPersonCamera() const { return FirstPersonCamera; }

	UFUNCTION(BlueprintPure, Category = "NewFPS")
	UNFInteractionComponent* GetInteractionComponent() const { return InteractionComponent; }

	/** Never null: falls back to the UNFRulesData class defaults (the spec's starting values). */
	UFUNCTION(BlueprintPure, Category = "NewFPS")
	const UNFRulesData* GetRules() const;

	/** The firearm in hand cannot be fired because of the water depth or the strongbox (see UNFWeaponComponent::EvaluateUse). */
	UFUNCTION(BlueprintPure, Category = "NewFPS")
	bool IsWeaponBlockedByWater() const;

	UFUNCTION(BlueprintPure, Category = "NewFPS")
	ENFLifeState GetLifeState() const { return LifeState; }

	/** Seconds a downed player has left before the lifeline pulls them out. */
	UFUNCTION(BlueprintPure, Category = "NewFPS")
	float GetBleedoutRemaining() const { return BleedoutRemaining; }

	UFUNCTION(BlueprintPure, Category = "NewFPS")
	bool IsCarrying() const;

	/** Server only: get a downed player back up. Returns false when the player is not downed. */
	bool AuthReviveFromDowned(float NewHealth, float InvulnerableSeconds);

	/** Called by the interaction component on every machine when the strongbox is picked up or put down. */
	void HandleCarryStateChanged(bool bNowCarrying);

protected:
	// ---- Components ----

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	/** First-person arms, visible to the owner only. Assign the mesh and AnimBP in the character Blueprint. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> FirstPersonArms;

	/** First-person weapon (skeletal variant). Mesh comes from UNFWeaponData::WeaponSkeletalMesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> FirstPersonWeaponSkeletal;

	/** First-person weapon (static variant). Mesh comes from UNFWeaponData::WeaponStaticMesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> FirstPersonWeaponStatic;

	/** Grey-box stand-in so other players are visible before a third-person mesh exists. Clear its mesh once one does. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PlaceholderBody;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNFWeaponComponent> WeaponComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNFHealthComponent> HealthComponent;

	/** How deep the character stands in the tide. Evaluated locally on every machine; not replicated. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNFWetnessComponent> WetnessComponent;

	/** Hold-to-interact (pick up, revive) and strongbox carrying. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNFInteractionComponent> InteractionComponent;

	/**
	 * Stand-in for the carried strongbox in first person: lower left corner of the view, owner only
	 * (spec 2.3: the load must read without a HUD). Replace with the real arm/box pose when it exists.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CarryProxyFirstPerson;

	/** Stand-in for the carried strongbox as other players see it. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CarryProxyThirdPerson;

	// ---- Data ----

	/** Movement tuning. Leave empty to use the class defaults of UNFMovementData. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Movement")
	TObjectPtr<UNFMovementData> MovementData;

	/** Health, downed/revive, strongbox and interaction numbers. Leave empty to use the class defaults of UNFRulesData. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Rules")
	TObjectPtr<UNFRulesData> RulesData;

	/** Socket on FirstPersonArms the weapon attaches to. If it does not exist the weapon hangs from the camera. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Presentation")
	FName WeaponAttachSocketName = TEXT("GripPoint");

	/** Weapon placement relative to the camera when there are no arms / no socket. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Presentation")
	FTransform WeaponCameraOffset;

	// ---- View ----

	/** Hip-fire horizontal FOV. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|View", meta = (ClampMin = "60.0", ClampMax = "120.0"))
	float DefaultFOV = 95.f;

	/** Degrees per unit of mouse delta. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|View", meta = (ClampMin = "0.01"))
	float MouseSensitivity = 1.f;

	/** Degrees per second at full stick deflection (X = yaw, Y = pitch). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|View")
	FVector2D GamepadLookRate = FVector2D(220.0, 140.0);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|View")
	bool bInvertLookY = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|View", meta = (ClampMin = "0.0"))
	float CrouchCameraInterpSpeed = 12.f;

	/** The camera sits this much lower while downed (cm). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|View", meta = (ClampMin = "0.0"))
	float DownedCameraDrop = 100.f;

	// ---- Input assets (create in the editor and assign in the character Blueprint) ----

	/**
	 * When an input asset below is left empty, a keyboard/mouse + gamepad default is built in code so the
	 * prototype is playable without any assets. Assigning DefaultMappingContext turns the fallback off.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	/** Axis2D: X = right, Y = forward. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Axis2D mouse delta: X = right, Y = up. Do not add a Negate modifier; use bInvertLookY. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Input")
	TObjectPtr<UInputAction> LookAction;

	/** Axis2D stick position (rate based): X = right, Y = up. Optional. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Input")
	TObjectPtr<UInputAction> LookStickAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Input")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Input")
	TObjectPtr<UInputAction> ReloadAction;

	/** Raise the other firearm. While carrying a strongbox this drops it instead. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Input")
	TObjectPtr<UInputAction> SwitchWeaponAction;

	/** Purser directly. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Input")
	TObjectPtr<UInputAction> SlotOneAction;

	/** Gaffer directly. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Input")
	TObjectPtr<UInputAction> SlotTwoAction;

	/** Kedge swing: its own button, no weapon switch. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Input")
	TObjectPtr<UInputAction> MeleeAction;

	/** Hold: pick up a strongbox / revive a teammate. Press while carrying: drop. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Input")
	TObjectPtr<UInputAction> InteractAction;

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_LifeState();

private:
	void ApplyMovementData();
	void BuildFallbackInput();
	void HandleWeaponVisualsReady();
	void AddLook(float YawDelta, float PitchDelta);

	UFUNCTION()
	void HandleDeath(UNFHealthComponent* InHealthComponent, AController* Killer);

	UFUNCTION()
	void HandleWetnessChanged(UNFWetnessComponent* InWetnessComponent, ENFWetness NewWetness, ENFWetness OldWetness);

	/** Server only, on a timer while the head is under water. */
	void ApplyDrownDamage();

	/** Apply the presentation and locks of LifeState on this machine. */
	void ApplyLifeState();
	void AuthEnterDowned();
	void AuthEnterOut();
	/** Server only, on a timer while downed. */
	void AuthTickBleedout();
	void RemoveInputMappingLocal();
	void UpdatePlaceholderBody();

	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_LookStick(const FInputActionValue& Value);
	void Input_JumpStarted();
	void Input_JumpCompleted();
	void Input_CrouchStarted();
	void Input_CrouchCompleted();
	void Input_SprintStarted();
	void Input_SprintCompleted();
	void Input_FireStarted();
	void Input_FireCompleted();
	void Input_AimStarted();
	void Input_AimCompleted();
	void Input_Reload();
	void Input_SwitchWeapon();
	void Input_SlotOne();
	void Input_SlotTwo();
	void Input_Melee();
	void Input_InteractStarted();
	void Input_InteractCompleted();
	void SelectSlotOrDrop(int32 Slot);

	UPROPERTY(Transient)
	TObjectPtr<UNFCharacterMovementComponent> NFMovement;

	UPROPERTY(ReplicatedUsing = OnRep_LifeState)
	ENFLifeState LifeState = ENFLifeState::Alive;

	UPROPERTY(Replicated)
	float BleedoutRemaining = 0.f;

	FTimerHandle DrownTimerHandle;
	FTimerHandle BleedoutTimerHandle;
	bool bSprintHeld = false;
	float CrouchEyeOffset = 0.f;
	float DownedEyeOffset = 0.f;
	/** Eye height above the capsule centre while standing (from the movement data). */
	float StandingBaseEyeHeight = 74.f;
};
