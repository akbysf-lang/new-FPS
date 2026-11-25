#include "NFCharacter.h"
#include "NewFPS.h"
#include "NFCharacterMovementComponent.h"
#include "NFGameMode.h"
#include "NFHealthComponent.h"
#include "NFMovementData.h"
#include "NFTideData.h"
#include "NFWeaponComponent.h"
#include "NFWeaponData.h"
#include "NFWetnessComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace NFCharacter
{
	const double MaxViewPitch = 89.0;
}

ANFCharacter::ANFCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UNFCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(40.f, 96.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	BaseEyeHeight = 64.f;
	CrouchedEyeHeight = 40.f;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->SetFieldOfView(DefaultFOV);

	FirstPersonArms = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonArms"));
	FirstPersonArms->SetupAttachment(FirstPersonCamera);
	FirstPersonArms->SetOnlyOwnerSee(true);
	FirstPersonArms->SetCastShadow(false);
	FirstPersonArms->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FirstPersonWeaponSkeletal = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FirstPersonWeaponSkeletal"));
	FirstPersonWeaponSkeletal->SetupAttachment(FirstPersonCamera);
	FirstPersonWeaponSkeletal->SetOnlyOwnerSee(true);
	FirstPersonWeaponSkeletal->SetCastShadow(false);
	FirstPersonWeaponSkeletal->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FirstPersonWeaponStatic = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FirstPersonWeaponStatic"));
	FirstPersonWeaponStatic->SetupAttachment(FirstPersonCamera);
	FirstPersonWeaponStatic->SetOnlyOwnerSee(true);
	FirstPersonWeaponStatic->SetCastShadow(false);
	FirstPersonWeaponStatic->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponCameraOffset = FTransform(FRotator::ZeroRotator, FVector(40.f, 18.f, -18.f));

	// Third-person mesh: other players see it, the owner does not.
	GetMesh()->SetOwnerNoSee(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	PlaceholderBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderBody"));
	PlaceholderBody->SetupAttachment(GetCapsuleComponent());
	PlaceholderBody->SetRelativeScale3D(FVector(0.7f, 0.7f, 1.8f));
	PlaceholderBody->SetOwnerNoSee(true);
	PlaceholderBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CylinderMesh.Succeeded())
	{
		PlaceholderBody->SetStaticMesh(CylinderMesh.Object);
	}

	WeaponComponent = CreateDefaultSubobject<UNFWeaponComponent>(TEXT("WeaponComponent"));
	HealthComponent = CreateDefaultSubobject<UNFHealthComponent>(TEXT("HealthComponent"));
	WetnessComponent = CreateDefaultSubobject<UNFWetnessComponent>(TEXT("WetnessComponent"));

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
}

void ANFCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	NFMovement = Cast<UNFCharacterMovementComponent>(GetCharacterMovement());
	ApplyMovementData();

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddUniqueDynamic(this, &ANFCharacter::HandleDeath);
	}
	if (WeaponComponent)
	{
		// Bound before BeginPlay so an already-loaded weapon is not missed.
		WeaponComponent->OnVisualsReady.AddUObject(this, &ANFCharacter::HandleWeaponVisualsReady);
	}
	if (WetnessComponent)
	{
		WetnessComponent->OnWetnessChanged.AddUniqueDynamic(this, &ANFCharacter::HandleWetnessChanged);
	}
}

void ANFCharacter::ApplyMovementData()
{
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement)
	{
		return;
	}

	const UNFMovementData* Data = MovementData ? MovementData.Get() : GetDefault<UNFMovementData>();

	Movement->MaxWalkSpeed = Data->WalkSpeed;
	Movement->MaxWalkSpeedCrouched = Data->CrouchSpeed;
	Movement->MaxAcceleration = Data->MaxAcceleration;
	Movement->BrakingDecelerationWalking = Data->BrakingDeceleration;
	Movement->GroundFriction = Data->GroundFriction;
	Movement->JumpZVelocity = Data->JumpZVelocity;
	Movement->AirControl = Data->AirControl;
	Movement->GravityScale = Data->GravityScale;
	Movement->SetCrouchedHalfHeight(Data->CrouchedHalfHeight);

	if (NFMovement)
	{
		NFMovement->SprintSpeed = Data->SprintSpeed;
		NFMovement->AimWalkSpeedMultiplier = Data->AimWalkSpeedMultiplier;
	}
}

void ANFCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsLocallyControlled())
	{
		return;
	}

	// Predicted movement states. Fire and ADS take priority over sprint.
	if (NFMovement && WeaponComponent)
	{
		NFMovement->bWantsToAimWalk = WeaponComponent->IsAiming();
		NFMovement->bWantsToSprint = bSprintHeld && !WeaponComponent->IsAiming() && !WeaponComponent->IsTriggerHeld();
	}

	// Smooth the camera through the capsule resize of crouch/uncrouch.
	CrouchEyeOffset = FMath::FInterpTo(CrouchEyeOffset, 0.f, DeltaSeconds, CrouchCameraInterpSpeed);
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight + CrouchEyeOffset));

	if (WeaponComponent)
	{
		FirstPersonCamera->SetFieldOfView(WeaponComponent->GetDesiredFOV(DefaultFOV));
	}
}

void ANFCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	const float OldEyeHeight = BaseEyeHeight;
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	// The capsule centre dropped and the eye height changed in one frame; start from where the camera was.
	CrouchEyeOffset += (OldEyeHeight - BaseEyeHeight) + ScaledHalfHeightAdjust;
}

void ANFCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	const float OldEyeHeight = BaseEyeHeight;
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	CrouchEyeOffset += (OldEyeHeight - BaseEyeHeight) - ScaledHalfHeightAdjust;
}

// ---------------------------------------------------------------------------------------------
// Presentation
// ---------------------------------------------------------------------------------------------

void ANFCharacter::HandleWeaponVisualsReady()
{
	if (!WeaponComponent)
	{
		return;
	}

	const UNFWeaponData* Data = WeaponComponent->GetData();
	USkeletalMesh* SkeletalWeapon = Data->WeaponSkeletalMesh.Get();
	UStaticMesh* StaticWeapon = SkeletalWeapon ? nullptr : Data->WeaponStaticMesh.Get();

	FirstPersonWeaponSkeletal->SetSkeletalMeshAsset(SkeletalWeapon);
	FirstPersonWeaponStatic->SetStaticMesh(StaticWeapon);

	UMeshComponent* ActiveMesh = nullptr;
	if (SkeletalWeapon)
	{
		ActiveMesh = FirstPersonWeaponSkeletal;
	}
	else if (StaticWeapon)
	{
		ActiveMesh = FirstPersonWeaponStatic;
	}

	if (ActiveMesh)
	{
		// Arms socket when the arms have it, otherwise a fixed spot in front of the camera.
		const bool bUseArmsSocket = FirstPersonArms->GetSkeletalMeshAsset() != nullptr && FirstPersonArms->DoesSocketExist(WeaponAttachSocketName);
		if (bUseArmsSocket)
		{
			ActiveMesh->AttachToComponent(FirstPersonArms, FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
			ActiveMesh->SetRelativeTransform(Data->WeaponAttachOffset);
		}
		else
		{
			ActiveMesh->AttachToComponent(FirstPersonCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
			ActiveMesh->SetRelativeTransform(Data->WeaponAttachOffset * WeaponCameraOffset);
		}
	}

	WeaponComponent->SetVisualMesh(ActiveMesh);
}

void ANFCharacter::HandleDeath(UNFHealthComponent* InHealthComponent, AController* Killer)
{
	if (WeaponComponent)
	{
		WeaponComponent->StopFire();
		WeaponComponent->SetAiming(false);
	}
	bSprintHeld = false;
	GetWorldTimerManager().ClearTimer(DrownTimerHandle);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();
	SetActorHiddenInGame(true);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (PlayerController->IsLocalController())
		{
			DisableInput(PlayerController);

			// The respawned pawn adds its own context; do not leave this one behind.
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				if (DefaultMappingContext)
				{
					Subsystem->RemoveMappingContext(DefaultMappingContext);
				}
			}
		}
	}

	if (HasAuthority())
	{
		if (ANFGameMode* GameMode = GetWorld()->GetAuthGameMode<ANFGameMode>())
		{
			GameMode->NotifyPlayerDied(GetController(), Killer);
		}
	}
}

// ---------------------------------------------------------------------------------------------
// Tide / wetness
// ---------------------------------------------------------------------------------------------

void ANFCharacter::HandleWetnessChanged(UNFWetnessComponent* InWetnessComponent, ENFWetness NewWetness, ENFWetness OldWetness)
{
	// Runs on every machine from the same deterministic water height, so nothing here needs an RPC.
	// The speed penalty is not applied here: UNFCharacterMovementComponent::GetMaxSpeed reads the depth itself.
	const UNFTideData* TideData = InWetnessComponent->GetTideData();

	const bool bWeaponBlocked = NewWetness >= TideData->WeaponsDisabledFrom;
	if (WeaponComponent)
	{
		WeaponComponent->SetFireBlocked(bWeaponBlocked);
		if (bWeaponBlocked)
		{
			WeaponComponent->SetAiming(false);
		}
	}

	if (HasAuthority())
	{
		FTimerManager& TimerManager = GetWorldTimerManager();
		if (NewWetness == ENFWetness::Overhead)
		{
			if (!TimerManager.IsTimerActive(DrownTimerHandle))
			{
				TimerManager.SetTimer(DrownTimerHandle, this, &ANFCharacter::ApplyDrownDamage,
					FMath::Max(TideData->DrownDamageInterval, 0.1f), true, FMath::Max(TideData->DrownGraceSeconds, 0.f));
			}
		}
		else
		{
			TimerManager.ClearTimer(DrownTimerHandle);
		}
	}
}

void ANFCharacter::ApplyDrownDamage()
{
	if (!HealthComponent || HealthComponent->IsDead() || !WetnessComponent)
	{
		GetWorldTimerManager().ClearTimer(DrownTimerHandle);
		return;
	}

	FNFDamageInfo DamageInfo;
	DamageInfo.BaseDamage = WetnessComponent->GetTideData()->DrownDamagePerInterval;
	DamageInfo.HitLocation = GetActorLocation();
	DamageInfo.ShotDirection = FVector::UpVector;
	HealthComponent->ApplyDamage(DamageInfo);
}

bool ANFCharacter::IsWeaponBlockedByWater() const
{
	return WeaponComponent && WeaponComponent->IsFireBlocked();
}

// ---------------------------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------------------------

void ANFCharacter::PawnClientRestart()
{
	// The fallback must exist before Super creates and binds the input component.
	BuildFallbackInput();

	Super::PawnClientRestart();

	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->RemoveMappingContext(DefaultMappingContext);
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void ANFCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	BuildFallbackInput();

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!Input)
	{
		UE_LOG(LogNewFPS, Error, TEXT("%s: input component is not an EnhancedInputComponent. Check DefaultInput.ini (DefaultInputComponentClass)."), *GetName());
		return;
	}

	if (MoveAction)
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANFCharacter::Input_Move);
	}
	if (LookAction)
	{
		Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANFCharacter::Input_Look);
	}
	if (LookStickAction)
	{
		Input->BindAction(LookStickAction, ETriggerEvent::Triggered, this, &ANFCharacter::Input_LookStick);
	}
	if (JumpAction)
	{
		Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ANFCharacter::Input_JumpStarted);
		Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ANFCharacter::Input_JumpCompleted);
	}
	if (CrouchAction)
	{
		Input->BindAction(CrouchAction, ETriggerEvent::Started, this, &ANFCharacter::Input_CrouchStarted);
		Input->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ANFCharacter::Input_CrouchCompleted);
	}
	if (SprintAction)
	{
		Input->BindAction(SprintAction, ETriggerEvent::Started, this, &ANFCharacter::Input_SprintStarted);
		Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &ANFCharacter::Input_SprintCompleted);
	}
	if (FireAction)
	{
		Input->BindAction(FireAction, ETriggerEvent::Started, this, &ANFCharacter::Input_FireStarted);
		Input->BindAction(FireAction, ETriggerEvent::Completed, this, &ANFCharacter::Input_FireCompleted);
	}
	if (AimAction)
	{
		Input->BindAction(AimAction, ETriggerEvent::Started, this, &ANFCharacter::Input_AimStarted);
		Input->BindAction(AimAction, ETriggerEvent::Completed, this, &ANFCharacter::Input_AimCompleted);
	}
	if (ReloadAction)
	{
		Input->BindAction(ReloadAction, ETriggerEvent::Started, this, &ANFCharacter::Input_Reload);
	}
}

void ANFCharacter::BuildFallbackInput()
{
	// Any asset assigned in the Blueprint wins; the fallback only fills what is missing,
	// and only maps keys when there is no authored mapping context.
	const bool bBuildMappings = (DefaultMappingContext == nullptr);

	auto EnsureAction = [this](TObjectPtr<UInputAction>& Action, const TCHAR* Name, EInputActionValueType ValueType) -> bool
	{
		if (Action)
		{
			return false;
		}
		UInputAction* NewAction = NewObject<UInputAction>(this, FName(Name));
		NewAction->ValueType = ValueType;
		Action = NewAction;
		return true;
	};

	// Each call has a side effect, so they are separate statements (clang rejects bool | bool here).
	bool bCreatedAny = false;
	bCreatedAny |= EnsureAction(MoveAction, TEXT("IA_Fallback_Move"), EInputActionValueType::Axis2D);
	bCreatedAny |= EnsureAction(LookAction, TEXT("IA_Fallback_Look"), EInputActionValueType::Axis2D);
	bCreatedAny |= EnsureAction(LookStickAction, TEXT("IA_Fallback_LookStick"), EInputActionValueType::Axis2D);
	bCreatedAny |= EnsureAction(JumpAction, TEXT("IA_Fallback_Jump"), EInputActionValueType::Boolean);
	bCreatedAny |= EnsureAction(CrouchAction, TEXT("IA_Fallback_Crouch"), EInputActionValueType::Boolean);
	bCreatedAny |= EnsureAction(SprintAction, TEXT("IA_Fallback_Sprint"), EInputActionValueType::Boolean);
	bCreatedAny |= EnsureAction(FireAction, TEXT("IA_Fallback_Fire"), EInputActionValueType::Boolean);
	bCreatedAny |= EnsureAction(AimAction, TEXT("IA_Fallback_Aim"), EInputActionValueType::Boolean);
	bCreatedAny |= EnsureAction(ReloadAction, TEXT("IA_Fallback_Reload"), EInputActionValueType::Boolean);

	if (!bBuildMappings)
	{
		if (bCreatedAny)
		{
			UE_LOG(LogNewFPS, Warning, TEXT("%s: DefaultMappingContext is assigned but some Input Actions are empty; those actions have no key bindings."), *GetName());
		}
		return;
	}

	UE_LOG(LogNewFPS, Log, TEXT("%s: no input assets assigned, using the built-in fallback bindings."), *GetName());

	UInputMappingContext* Context = NewObject<UInputMappingContext>(this, TEXT("IMC_Fallback"));

	// Modifiers run in the order they are added.
	auto Map = [Context](UInputAction* Action, const FKey& Key, bool bNegate, bool bSwizzle, bool bDeadZone)
	{
		FEnhancedActionKeyMapping& Mapping = Context->MapKey(Action, Key);
		if (bDeadZone)
		{
			Mapping.Modifiers.Add(NewObject<UInputModifierDeadZone>(Context));
		}
		if (bNegate)
		{
			Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(Context));
		}
		if (bSwizzle)
		{
			// Default order YXZ: routes a key's X value into the Y axis.
			Mapping.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(Context));
		}
	};

	// Move: X = right, Y = forward.
	Map(MoveAction, EKeys::W, false, true, false);
	Map(MoveAction, EKeys::S, true, true, false);
	Map(MoveAction, EKeys::D, false, false, false);
	Map(MoveAction, EKeys::A, true, false, false);
	Map(MoveAction, EKeys::Gamepad_Left2D, false, false, true);

	Map(LookAction, EKeys::Mouse2D, false, false, false);
	Map(LookStickAction, EKeys::Gamepad_Right2D, false, false, true);

	Map(JumpAction, EKeys::SpaceBar, false, false, false);
	Map(JumpAction, EKeys::Gamepad_FaceButton_Bottom, false, false, false);
	Map(CrouchAction, EKeys::LeftControl, false, false, false);
	Map(CrouchAction, EKeys::Gamepad_FaceButton_Right, false, false, false);
	Map(SprintAction, EKeys::LeftShift, false, false, false);
	Map(SprintAction, EKeys::Gamepad_LeftThumbstick, false, false, false);
	Map(FireAction, EKeys::LeftMouseButton, false, false, false);
	Map(FireAction, EKeys::Gamepad_RightTrigger, false, false, false);
	Map(AimAction, EKeys::RightMouseButton, false, false, false);
	Map(AimAction, EKeys::Gamepad_LeftTrigger, false, false, false);
	Map(ReloadAction, EKeys::R, false, false, false);
	Map(ReloadAction, EKeys::Gamepad_FaceButton_Left, false, false, false);

	DefaultMappingContext = Context;
}

void ANFCharacter::AddLook(float YawDelta, float PitchDelta)
{
	if (!Controller || (YawDelta == 0.f && PitchDelta == 0.f))
	{
		return;
	}

	// Written straight into the control rotation: no input scaling, smoothing or acceleration in between.
	FRotator ControlRotation = Controller->GetControlRotation();
	ControlRotation.Yaw += YawDelta;
	ControlRotation.Pitch = FMath::ClampAngle(ControlRotation.Pitch + PitchDelta, -NFCharacter::MaxViewPitch, NFCharacter::MaxViewPitch);
	Controller->SetControlRotation(ControlRotation);

	if (WeaponComponent)
	{
		WeaponComponent->NotifyAimInput(YawDelta, PitchDelta);
	}
}

void ANFCharacter::Input_Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddMovementInput(GetActorForwardVector(), Axis.Y);
	AddMovementInput(GetActorRightVector(), Axis.X);
}

void ANFCharacter::Input_Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	const float Scale = MouseSensitivity * (WeaponComponent ? WeaponComponent->GetLookSensitivityScale(DefaultFOV) : 1.f);
	AddLook(Axis.X * Scale, Axis.Y * Scale * (bInvertLookY ? -1.f : 1.f));
}

void ANFCharacter::Input_LookStick(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	const float DeltaSeconds = GetWorld()->GetDeltaSeconds();
	const float Scale = DeltaSeconds * (WeaponComponent ? WeaponComponent->GetLookSensitivityScale(DefaultFOV) : 1.f);
	AddLook(Axis.X * GamepadLookRate.X * Scale, Axis.Y * GamepadLookRate.Y * Scale * (bInvertLookY ? -1.f : 1.f));
}

void ANFCharacter::Input_JumpStarted()
{
	// Jumping out of a crouch stands up first, as players expect.
	if (bIsCrouched)
	{
		UnCrouch();
	}
	Jump();
}

void ANFCharacter::Input_JumpCompleted()
{
	StopJumping();
}

void ANFCharacter::Input_CrouchStarted()
{
	Crouch();
}

void ANFCharacter::Input_CrouchCompleted()
{
	UnCrouch();
}

void ANFCharacter::Input_SprintStarted()
{
	bSprintHeld = true;
}

void ANFCharacter::Input_SprintCompleted()
{
	bSprintHeld = false;
}

void ANFCharacter::Input_FireStarted()
{
	if (WeaponComponent)
	{
		WeaponComponent->StartFire();
	}
}

void ANFCharacter::Input_FireCompleted()
{
	if (WeaponComponent)
	{
		WeaponComponent->StopFire();
	}
}

void ANFCharacter::Input_AimStarted()
{
	if (WeaponComponent && !IsWeaponBlockedByWater())
	{
		WeaponComponent->SetAiming(true);
	}
}

void ANFCharacter::Input_AimCompleted()
{
	if (WeaponComponent)
	{
		WeaponComponent->SetAiming(false);
	}
}

void ANFCharacter::Input_Reload()
{
	if (WeaponComponent)
	{
		WeaponComponent->StartReload();
	}
}
