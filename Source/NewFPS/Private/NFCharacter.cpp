#include "NFCharacter.h"
#include "NewFPS.h"
#include "NFCharacterMovementComponent.h"
#include "NFGameMode.h"
#include "NFHealthComponent.h"
#include "NFInteractionComponent.h"
#include "NFMovementData.h"
#include "NFRulesData.h"
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
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace NFCharacter
{
	const double MaxViewPitch = 89.0;
	/** Seconds between bleed-out steps on the server. */
	const float BleedoutInterval = 0.25f;
	/** Height of the grey-box body while downed (cm). */
	const float DownedBodyHeight = 60.f;
}

ANFCharacter::ANFCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UNFCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	// Spec 2.1: radius 34, half height 90 (180 tall). Edit the capsule here or in the character Blueprint, not in
	// the movement data: the engine restores the standing capsule from the class defaults after a crouch.
	GetCapsuleComponent()->InitCapsuleSize(34.f, 90.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Relative to the capsule centre: eyes 164 above the floor standing (90 + 74), 104 crouched (55 + 49).
	// PostInitializeComponents recomputes both from the movement data.
	BaseEyeHeight = 74.f;
	CrouchedEyeHeight = 49.f;

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
	PlaceholderBody->SetRelativeScale3D(FVector(0.68f, 0.68f, 1.8f));
	PlaceholderBody->SetOwnerNoSee(true);
	PlaceholderBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (CylinderMesh.Succeeded())
	{
		PlaceholderBody->SetStaticMesh(CylinderMesh.Object);
	}

	WeaponComponent = CreateDefaultSubobject<UNFWeaponComponent>(TEXT("WeaponComponent"));
	HealthComponent = CreateDefaultSubobject<UNFHealthComponent>(TEXT("HealthComponent"));
	WetnessComponent = CreateDefaultSubobject<UNFWetnessComponent>(TEXT("WetnessComponent"));
	InteractionComponent = CreateDefaultSubobject<UNFInteractionComponent>(TEXT("InteractionComponent"));

	// Carried strongbox stand-ins (engine cube, 100 cm). Hidden until a strongbox is picked up.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));

	CarryProxyFirstPerson = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarryProxyFirstPerson"));
	CarryProxyFirstPerson->SetupAttachment(FirstPersonCamera);
	// Lower left corner of the view, clear of the crosshair: roughly 6% of the screen at FOV 95.
	CarryProxyFirstPerson->SetRelativeLocation(FVector(45.f, -34.f, -24.f));
	CarryProxyFirstPerson->SetRelativeScale3D(FVector(0.28f, 0.20f, 0.16f));
	CarryProxyFirstPerson->SetOnlyOwnerSee(true);
	CarryProxyFirstPerson->SetCastShadow(false);
	CarryProxyFirstPerson->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CarryProxyFirstPerson->SetVisibility(false);

	CarryProxyThirdPerson = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CarryProxyThirdPerson"));
	CarryProxyThirdPerson->SetupAttachment(GetCapsuleComponent());
	// Under the left arm, real size (60 x 40 x 40).
	CarryProxyThirdPerson->SetRelativeLocation(FVector(15.f, -42.f, 10.f));
	CarryProxyThirdPerson->SetRelativeScale3D(FVector(0.6f, 0.4f, 0.4f));
	CarryProxyThirdPerson->SetOwnerNoSee(true);
	CarryProxyThirdPerson->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CarryProxyThirdPerson->SetVisibility(false);

	if (CubeMesh.Succeeded())
	{
		CarryProxyFirstPerson->SetStaticMesh(CubeMesh.Object);
		CarryProxyThirdPerson->SetStaticMesh(CubeMesh.Object);
	}

	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
}

void ANFCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANFCharacter, LifeState);
	DOREPLIFETIME(ANFCharacter, BleedoutRemaining);
}

const UNFRulesData* ANFCharacter::GetRules() const
{
	return RulesData ? RulesData.Get() : GetDefault<UNFRulesData>();
}

bool ANFCharacter::IsCarrying() const
{
	return InteractionComponent && InteractionComponent->IsCarrying();
}

void ANFCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	NFMovement = Cast<UNFCharacterMovementComponent>(GetCharacterMovement());
	ApplyMovementData();

	if (HealthComponent)
	{
		// Before the component's BeginPlay, which fills the health up to the maximum.
		HealthComponent->SetMaxHealth(GetRules()->MaxHealth, /*bRefill*/ true);
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

void ANFCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && HealthComponent)
	{
		// Spec 6.1: after 5 s without damage, 8 health/s, but only up to 50.
		const UNFRulesData* Rules = GetRules();
		HealthComponent->ConfigureRegen(Rules->RegenDelay, Rules->RegenPerSecond, Rules->RegenCap);
		HealthComponent->SetRegenEnabled(Rules->RegenPerSecond > 0.f);
	}

	// A late joiner receives LifeState before BeginPlay.
	ApplyLifeState();
	HandleCarryStateChanged(IsCarrying());
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
	Movement->MaxStepHeight = Data->MaxStepHeight;
	Movement->SetWalkableFloorAngle(Data->WalkableFloorAngle);
	Movement->SetCrouchedHalfHeight(Data->CrouchedHalfHeight);

	if (NFMovement)
	{
		NFMovement->SprintSpeed = Data->SprintSpeed;
		NFMovement->AimWalkSpeedMultiplier = Data->AimWalkSpeedMultiplier;
		NFMovement->BaseJumpZVelocity = Data->JumpZVelocity;
		NFMovement->CarryWalkSpeed = Data->CarryWalkSpeed;
		NFMovement->CarryCrouchSpeed = Data->CarryCrouchSpeed;
		NFMovement->CarryJumpZVelocity = Data->CarryJumpZVelocity;
		NFMovement->DownedCrawlSpeed = Data->DownedCrawlSpeed;
	}

	// The data gives eye heights above the floor; the engine wants them relative to the capsule centre.
	const float StandingHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	StandingBaseEyeHeight = Data->StandingEyeHeight - StandingHalfHeight;
	CrouchedEyeHeight = Data->CrouchedEyeHeight - Data->CrouchedHalfHeight;
	BaseEyeHeight = StandingBaseEyeHeight;
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight));

	UpdatePlaceholderBody();
}

void ANFCharacter::RecalculateBaseEyeHeight()
{
	// The engine's version reads the standing eye height from the class defaults; ours comes from the movement data.
	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	BaseEyeHeight = (Movement && Movement->IsCrouching()) ? CrouchedEyeHeight : StandingBaseEyeHeight;
}

void ANFCharacter::UpdatePlaceholderBody()
{
	if (!PlaceholderBody)
	{
		return;
	}

	// Engine cylinder: 100 cm across and tall. Standing: fills the capsule. Downed: a low slab on the floor.
	const float Radius = GetCapsuleComponent()->GetUnscaledCapsuleRadius();
	const float HalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
	const bool bLow = LifeState == ENFLifeState::Downed;
	const float BodyHeight = bLow ? NFCharacter::DownedBodyHeight : HalfHeight * 2.f;

	PlaceholderBody->SetRelativeScale3D(FVector(Radius * 0.02f, Radius * 0.02f, BodyHeight * 0.01f));
	PlaceholderBody->SetRelativeLocation(FVector(0.f, 0.f, bLow ? BodyHeight * 0.5f - HalfHeight : 0.f));
}

void ANFCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsLocallyControlled())
	{
		return;
	}

	// Predicted movement states. Fire, ADS and the Kedge take priority over sprint (spec 2.2, 4.3).
	if (NFMovement && WeaponComponent)
	{
		NFMovement->bWantsToAimWalk = WeaponComponent->IsAiming();
		NFMovement->bWantsToSprint = bSprintHeld
			&& !WeaponComponent->IsAiming()
			&& !WeaponComponent->IsTriggerHeld()
			&& !WeaponComponent->IsMeleeActive();
	}

	// Smooth the camera through the capsule resize of crouch/uncrouch and through going down / getting up.
	CrouchEyeOffset = FMath::FInterpTo(CrouchEyeOffset, 0.f, DeltaSeconds, CrouchCameraInterpSpeed);
	DownedEyeOffset = FMath::FInterpTo(DownedEyeOffset, LifeState == ENFLifeState::Downed ? -DownedCameraDrop : 0.f, DeltaSeconds, CrouchCameraInterpSpeed);
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight + CrouchEyeOffset + DownedEyeOffset));

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

// ---------------------------------------------------------------------------------------------
// Life cycle: Alive -> Downed -> (revived | Out). Spec 6.2.
// ---------------------------------------------------------------------------------------------

void ANFCharacter::HandleDeath(UNFHealthComponent* InHealthComponent, AController* Killer)
{
	// "Dead" on the health component means "at 0 health": the player goes down instead of dying.
	// Clients follow through OnRep_LifeState.
	if (HasAuthority() && LifeState == ENFLifeState::Alive)
	{
		AuthEnterDowned();
	}
}

void ANFCharacter::AuthEnterDowned()
{
	LifeState = ENFLifeState::Downed;
	BleedoutRemaining = GetRules()->BleedoutSeconds;

	if (InteractionComponent)
	{
		// Spec 2.3: the strongbox is only let go when its carrier goes down.
		InteractionComponent->AuthCancelInteraction();
		InteractionComponent->AuthDropCarried();
	}

	GetWorldTimerManager().ClearTimer(DrownTimerHandle);
	GetWorldTimerManager().SetTimer(BleedoutTimerHandle, this, &ANFCharacter::AuthTickBleedout, NFCharacter::BleedoutInterval, true);

	ApplyLifeState();
	ForceNetUpdate();

	if (ANFGameMode* GameMode = GetWorld()->GetAuthGameMode<ANFGameMode>())
	{
		GameMode->NotifyPlayerDowned(this);
	}
}

void ANFCharacter::AuthTickBleedout()
{
	if (LifeState != ENFLifeState::Downed)
	{
		GetWorldTimerManager().ClearTimer(BleedoutTimerHandle);
		return;
	}

	// Lying in the water: the tide collects three times as fast.
	const UNFRulesData* Rules = GetRules();
	const bool bInWater = WetnessComponent && WetnessComponent->GetWaterDepth() >= Rules->WetBleedoutDepth;
	BleedoutRemaining -= NFCharacter::BleedoutInterval * (bInWater ? Rules->WetBleedoutMultiplier : 1.f);

	if (BleedoutRemaining <= 0.f)
	{
		BleedoutRemaining = 0.f;
		AuthEnterOut();
	}
}

void ANFCharacter::AuthEnterOut()
{
	if (!HasAuthority() || LifeState == ENFLifeState::Out)
	{
		return;
	}

	LifeState = ENFLifeState::Out;
	GetWorldTimerManager().ClearTimer(BleedoutTimerHandle);
	ApplyLifeState();
	ForceNetUpdate();

	if (ANFGameMode* GameMode = GetWorld()->GetAuthGameMode<ANFGameMode>())
	{
		GameMode->NotifyPlayerOut(this);
	}
}

bool ANFCharacter::AuthReviveFromDowned(float NewHealth, float InvulnerableSeconds)
{
	if (!HasAuthority() || LifeState != ENFLifeState::Downed || !HealthComponent)
	{
		return false;
	}

	GetWorldTimerManager().ClearTimer(BleedoutTimerHandle);
	HealthComponent->ReviveTo(NewHealth);
	HealthComponent->SetInvulnerableFor(InvulnerableSeconds);

	LifeState = ENFLifeState::Alive;
	BleedoutRemaining = 0.f;
	ApplyLifeState();
	ForceNetUpdate();

	if (WetnessComponent)
	{
		// Got up with the head under water: restart the drowning clock.
		HandleWetnessChanged(WetnessComponent, WetnessComponent->GetWetness(), WetnessComponent->GetWetness());
	}
	return true;
}

void ANFCharacter::OnRep_LifeState()
{
	ApplyLifeState();
}

void ANFCharacter::ApplyLifeState()
{
	const bool bAlive = LifeState == ENFLifeState::Alive;
	const bool bOut = LifeState == ENFLifeState::Out;

	if (WeaponComponent)
	{
		if (!bAlive)
		{
			WeaponComponent->StopFire();
			WeaponComponent->SetAiming(false);
		}
		// Downed: no firing, no reload, no Kedge, no weapon switch.
		WeaponComponent->SetFireBlocked(!bAlive);
	}
	if (NFMovement)
	{
		// Downed players crawl (UNFMovementData::DownedCrawlSpeed).
		NFMovement->bDowned = (LifeState == ENFLifeState::Downed);
	}
	if (!bAlive)
	{
		bSprintHeld = false;
		UnCrouch();
	}

	UpdatePlaceholderBody();

	// Out: the lifeline pulled the player up; this pawn is only a husk until the respawn replaces it.
	SetActorHiddenInGame(bOut);
	GetCapsuleComponent()->SetCollisionEnabled(bOut ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
	if (bOut)
	{
		GetCharacterMovement()->DisableMovement();

		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			if (PlayerController->IsLocalController())
			{
				DisableInput(PlayerController);
				RemoveInputMappingLocal();
			}
		}
	}
}

void ANFCharacter::RemoveInputMappingLocal()
{
	// The respawned pawn adds its own context; do not leave this one behind.
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController || !DefaultMappingContext)
	{
		return;
	}
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
	{
		Subsystem->RemoveMappingContext(DefaultMappingContext);
	}
}

void ANFCharacter::HandleCarryStateChanged(bool bNowCarrying)
{
	// Spec 2.3: 400 cm/s, no sprint, low jump; Purser only, hip-fire only, no reload; no Kedge.
	if (NFMovement)
	{
		NFMovement->bCarrying = bNowCarrying;
	}
	if (WeaponComponent)
	{
		WeaponComponent->SetCarrying(bNowCarrying);
		if (bNowCarrying && HasAuthority())
		{
			// Gaffer in hand when the box comes up: the Purser replaces it.
			WeaponComponent->AuthEnsureUsableSlot();
		}
	}
	if (CarryProxyFirstPerson)
	{
		CarryProxyFirstPerson->SetVisibility(bNowCarrying);
	}
	if (CarryProxyThirdPerson)
	{
		CarryProxyThirdPerson->SetVisibility(bNowCarrying);
	}
}

// ---------------------------------------------------------------------------------------------
// Tide / wetness
// ---------------------------------------------------------------------------------------------

void ANFCharacter::HandleWetnessChanged(UNFWetnessComponent* InWetnessComponent, ENFWetness NewWetness, ENFWetness OldWetness)
{
	// Runs on every machine from the same deterministic water height, so nothing here needs an RPC.
	// Not handled here on purpose:
	//  - the speed/jump penalty: UNFCharacterMovementComponent reads the depth itself;
	//  - the weapon rules: UNFWeaponComponent::EvaluateUse reads the depth itself (the lock is bound to depth,
	//    not to this state: Gaffer down at 100 cm, every firearm down at ChestDepth 135 cm).
	const UNFTideData* TideData = InWetnessComponent->GetTideData();

	if (!HasAuthority())
	{
		return;
	}

	FTimerManager& TimerManager = GetWorldTimerManager();
	if (NewWetness == ENFWetness::Overhead && LifeState == ENFLifeState::Alive)
	{
		// Head under water: whatever is carried is let go (spec 3.3).
		if (InteractionComponent)
		{
			InteractionComponent->AuthCancelInteraction();
			InteractionComponent->AuthDropCarried();
		}

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

void ANFCharacter::ApplyDrownDamage()
{
	if (!HealthComponent || HealthComponent->IsDead() || !WetnessComponent)
	{
		GetWorldTimerManager().ClearTimer(DrownTimerHandle);
		return;
	}

	FNFDamageInfo DamageInfo;
	DamageInfo.BaseDamage = WetnessComponent->GetTideData()->DrownDamagePerInterval;
	DamageInfo.DamageType = ENFDamageType::Drown;
	DamageInfo.HitLocation = GetActorLocation();
	DamageInfo.ShotDirection = FVector::UpVector;
	HealthComponent->ApplyDamage(DamageInfo);
}

bool ANFCharacter::IsWeaponBlockedByWater() const
{
	return WeaponComponent && WeaponComponent->IsFirearmUnusable();
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
	if (SwitchWeaponAction)
	{
		Input->BindAction(SwitchWeaponAction, ETriggerEvent::Started, this, &ANFCharacter::Input_SwitchWeapon);
	}
	if (SlotOneAction)
	{
		Input->BindAction(SlotOneAction, ETriggerEvent::Started, this, &ANFCharacter::Input_SlotOne);
	}
	if (SlotTwoAction)
	{
		Input->BindAction(SlotTwoAction, ETriggerEvent::Started, this, &ANFCharacter::Input_SlotTwo);
	}
	if (MeleeAction)
	{
		Input->BindAction(MeleeAction, ETriggerEvent::Started, this, &ANFCharacter::Input_Melee);
	}
	if (InteractAction)
	{
		Input->BindAction(InteractAction, ETriggerEvent::Started, this, &ANFCharacter::Input_InteractStarted);
		Input->BindAction(InteractAction, ETriggerEvent::Completed, this, &ANFCharacter::Input_InteractCompleted);
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
	bCreatedAny |= EnsureAction(SwitchWeaponAction, TEXT("IA_Fallback_SwitchWeapon"), EInputActionValueType::Boolean);
	bCreatedAny |= EnsureAction(SlotOneAction, TEXT("IA_Fallback_SlotOne"), EInputActionValueType::Boolean);
	bCreatedAny |= EnsureAction(SlotTwoAction, TEXT("IA_Fallback_SlotTwo"), EInputActionValueType::Boolean);
	bCreatedAny |= EnsureAction(MeleeAction, TEXT("IA_Fallback_Melee"), EInputActionValueType::Boolean);
	bCreatedAny |= EnsureAction(InteractAction, TEXT("IA_Fallback_Interact"), EInputActionValueType::Boolean);

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
	Map(SwitchWeaponAction, EKeys::Q, false, false, false);
	Map(SwitchWeaponAction, EKeys::Gamepad_FaceButton_Top, false, false, false);
	Map(SlotOneAction, EKeys::One, false, false, false);
	Map(SlotTwoAction, EKeys::Two, false, false, false);
	Map(MeleeAction, EKeys::V, false, false, false);
	Map(MeleeAction, EKeys::Gamepad_RightThumbstick, false, false, false);
	Map(InteractAction, EKeys::E, false, false, false);
	Map(InteractAction, EKeys::Gamepad_DPad_Down, false, false, false);

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
	if (GetCharacterMovement()->IsCrouching())
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
	if (LifeState == ENFLifeState::Alive)
	{
		Crouch();
	}
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
	// The intent is always recorded; UNFWeaponComponent::IsAiming decides whether it is allowed right now
	// (not waist deep, not carrying, not mid-swing), so ADS comes back by itself when the block ends.
	if (WeaponComponent)
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

void ANFCharacter::SelectSlotOrDrop(int32 Slot)
{
	// Spec 2.3: switching weapons is the other way to let go of the strongbox. The weapon itself does not
	// change on that press (only the Purser works one-handed anyway).
	if (IsCarrying())
	{
		InteractionComponent->RequestDrop();
		return;
	}

	if (WeaponComponent)
	{
		if (Slot < 0)
		{
			WeaponComponent->SwitchWeapon();
		}
		else
		{
			WeaponComponent->SelectSlot(Slot);
		}
	}
}

void ANFCharacter::Input_SwitchWeapon()
{
	SelectSlotOrDrop(-1);
}

void ANFCharacter::Input_SlotOne()
{
	SelectSlotOrDrop(0);
}

void ANFCharacter::Input_SlotTwo()
{
	SelectSlotOrDrop(1);
}

void ANFCharacter::Input_Melee()
{
	if (WeaponComponent)
	{
		WeaponComponent->StartMelee();
	}
}

void ANFCharacter::Input_InteractStarted()
{
	if (InteractionComponent && LifeState == ENFLifeState::Alive)
	{
		InteractionComponent->BeginInteract();
	}
}

void ANFCharacter::Input_InteractCompleted()
{
	if (InteractionComponent)
	{
		InteractionComponent->EndInteract();
	}
}
