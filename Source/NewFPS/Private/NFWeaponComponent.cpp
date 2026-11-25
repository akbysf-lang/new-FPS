#include "NFWeaponComponent.h"
#include "NewFPS.h"
#include "NFHealthComponent.h"
#include "NFTideData.h"
#include "NFWeaponData.h"
#include "NFWetnessComponent.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/AssetManager.h"
#include "Engine/HitResult.h"
#include "Engine/OverlapResult.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

static TAutoConsoleVariable<int32> CVarNFDebugShots(
	TEXT("nf.Weapon.DebugShots"),
	1,
	TEXT("Draw a debug line and impact point for every shot (grey-box tracer). 0 = off."));

namespace NFWeapon
{
	/** Hard cap on shots released in a single frame after a hitch. */
	const int32 MaxShotsPerFrame = 8;
	const double MaxViewPitch = 89.0;
	/** Depth reported when the owner has no wetness component or the level has no tide. */
	const float NoWaterDepth = -100000.f;
	/** Hits closer than this skip the server's spread-cone test (muzzle obstruction changes the direction). */
	const float ConeCheckMinDistance = 200.f;
}

UNFWeaponComponent::UNFWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UNFWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UNFWeaponComponent, RepAmmoInMagazine, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UNFWeaponComponent, RepReserveAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(UNFWeaponComponent, bAiming, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(UNFWeaponComponent, bReloading, COND_SkipOwner);
	DOREPLIFETIME(UNFWeaponComponent, RepActiveSlot);
}

void UNFWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerWetness = GetOwner() ? GetOwner()->FindComponentByClass<UNFWetnessComponent>() : nullptr;

	// Every machine starts from the same guess (full magazines), so the owner's reconcile in Tick does not
	// zero the ammo before the first replication.
	for (int32 Slot = 0; Slot < NumSlots; ++Slot)
	{
		const UNFWeaponData* SlotData = GetSlotData(Slot);
		StoredMagazine[Slot] = SlotData->MagazineSize;
		StoredReserve[Slot] = SlotData->bInfiniteReserve ? 0 : SlotData->StartingReserveAmmo;
	}

	// A client can receive the slot before BeginPlay.
	ActiveSlot = OwnerHasAuthority() ? 0 : FMath::Clamp<int32>(RepActiveSlot, 0, NumSlots - 1);
	SetAmmo(StoredMagazine[ActiveSlot], StoredReserve[ActiveSlot]);
	if (OwnerHasAuthority())
	{
		RepActiveSlot = static_cast<uint8>(ActiveSlot);
	}
	else
	{
		RepAmmoInMagazine = AmmoInMagazine;
		RepReserveAmmo = ReserveAmmo;
	}

	if (GetNetMode() != NM_DedicatedServer)
	{
		RequestVisualAssets();
	}
}

void UNFWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
		World->GetTimerManager().ClearTimer(MeleeHitTimerHandle);
	}
	VisualAssetsHandle.Reset();

	Super::EndPlay(EndPlayReason);
}

void UNFWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const UNFWeaponData* Data = GetData();

	// ADS blend runs everywhere so third-person presentation can read it later.
	AdsAlpha = FMath::FInterpConstantTo(AdsAlpha, IsAiming() ? 1.f : 0.f, DeltaTime, 1.f / FMath::Max(Data->AdsTime, 0.01f));

	if (!IsOwnerLocallyControlled())
	{
		return;
	}

	SpreadHeat = FMath::Max(0.f, SpreadHeat - Data->SpreadRecoveryPerSecond * DeltaTime);

	// Waist deep with the Gaffer up (or a strongbox just picked up): go to the weapon that still works.
	if (!IsSwitching() && !IsMeleeActive() && !IsOwnerDead() && !EvaluateUse(Data).bUsable)
	{
		const int32 OtherSlot = (ActiveSlot + 1) % NumSlots;
		if (EvaluateUse(GetSlotData(OtherSlot)).bUsable)
		{
			SelectSlot(OtherSlot);
		}
	}

	UpdateFiring(DeltaTime);
	UpdateRecoil(DeltaTime);

	// Adopt the server's slot and ammo once our predictions have had time to be acknowledged.
	if (!OwnerHasAuthority())
	{
		const float Now = GetWorldTime();
		if (ActiveSlot != RepActiveSlot)
		{
			if (Now - LastSlotRequestTime > AmmoReconcileDelay)
			{
				ApplySlot(RepActiveSlot);
			}
		}
		else if (!bReloading
			&& Now - LastPredictedAmmoChangeTime > AmmoReconcileDelay
			&& (AmmoInMagazine != RepAmmoInMagazine || ReserveAmmo != RepReserveAmmo))
		{
			SetAmmo(RepAmmoInMagazine, RepReserveAmmo);
		}
	}
}

// ---------------------------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------------------------

const UNFWeaponData* UNFWeaponComponent::GetData() const
{
	return GetSlotData(ActiveSlot);
}

const UNFWeaponData* UNFWeaponComponent::GetSlotData(int32 Slot) const
{
	if (Loadout.IsValidIndex(Slot) && Loadout[Slot])
	{
		return Loadout[Slot].Get();
	}
	if (Slot == 1)
	{
		return GetDefault<UNFGafferData>();
	}
	return GetDefault<UNFPurserData>();
}

const UNFMeleeData* UNFWeaponComponent::GetMeleeData() const
{
	return MeleeData ? MeleeData.Get() : GetDefault<UNFMeleeData>();
}

float UNFWeaponComponent::GetWorldTime() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetTimeSeconds() : 0.f;
}

float UNFWeaponComponent::GetOwnerWaterDepth() const
{
	return OwnerWetness ? OwnerWetness->GetWaterDepth() : NFWeapon::NoWaterDepth;
}

const UNFTideData* UNFWeaponComponent::GetTideRules() const
{
	return OwnerWetness ? OwnerWetness->GetTideData() : GetDefault<UNFTideData>();
}

FNFWeaponUse UNFWeaponComponent::EvaluateUse(const UNFWeaponData* Data, float DepthSlack) const
{
	FNFWeaponUse Use;
	if (!Data)
	{
		Use.bUsable = false;
		Use.bCanReload = false;
		return Use;
	}

	// Spec 3.3: the firearm lock follows the depth itself, not the (hysteresis-smoothed) wetness state.
	const UNFTideData* Tide = GetTideRules();
	const float Depth = GetOwnerWaterDepth();
	const bool bWaistDeep = Depth >= Tide->WaistDepth;

	Use.bUsable = Depth < Tide->GetFirearmDepthLimit(Data->MaxUsableWetness) + DepthSlack
		&& (!bCarrying || Data->bUsableWhileCarrying);
	Use.bHipOnly = bWaistDeep || bCarrying;
	// One hand holds the strongbox: no reload while carrying (spec 2.3).
	Use.bCanReload = Use.bUsable && !bCarrying;
	Use.SpreadScale = bWaistDeep ? Tide->WaistSpreadMultiplier : 1.f;
	Use.ReloadTimeScale = bWaistDeep ? Tide->WaistReloadTimeMultiplier : 1.f;
	return Use;
}

bool UNFWeaponComponent::IsFirearmUnusable() const
{
	return !EvaluateUse(GetData()).bUsable;
}

bool UNFWeaponComponent::IsSwitching() const
{
	return GetWorldTime() < SwitchEndTime;
}

bool UNFWeaponComponent::IsMeleeActive() const
{
	return GetWorldTime() < MeleeEndTime;
}

bool UNFWeaponComponent::IsAiming() const
{
	if (!bAiming || bReloading || bFireBlocked || IsSwitching() || IsMeleeActive())
	{
		return false;
	}
	const FNFWeaponUse Use = EvaluateUse(GetData());
	return Use.bUsable && !Use.bHipOnly;
}

void UNFWeaponComponent::SetFireBlocked(bool bNewBlocked)
{
	bFireBlocked = bNewBlocked;
	if (bFireBlocked)
	{
		bTriggerHeld = false;
		bSemiShotPending = false;
		CancelReloadInternal();
	}
}

void UNFWeaponComponent::SetCarrying(bool bNewCarrying)
{
	if (bCarrying == bNewCarrying)
	{
		return;
	}

	bCarrying = bNewCarrying;
	if (bCarrying)
	{
		// The reloading hand is now holding the strongbox.
		CancelReloadInternal();
	}
}

bool UNFWeaponComponent::IsFriendlyTarget(const AActor* Target) const
{
	if (bFriendlyFire)
	{
		return false;
	}
	const APawn* TargetPawn = Cast<APawn>(Target);
	return TargetPawn && TargetPawn->IsPlayerControlled();
}

APawn* UNFWeaponComponent::GetOwnerPawn() const
{
	return Cast<APawn>(GetOwner());
}

bool UNFWeaponComponent::IsOwnerLocallyControlled() const
{
	const APawn* OwnerPawn = GetOwnerPawn();
	return OwnerPawn && OwnerPawn->IsLocallyControlled();
}

bool UNFWeaponComponent::OwnerHasAuthority() const
{
	const AActor* Owner = GetOwner();
	return Owner && Owner->HasAuthority();
}

bool UNFWeaponComponent::IsOwnerDead() const
{
	const UNFHealthComponent* Health = UNFHealthComponent::FindHealthComponent(GetOwner());
	return Health && Health->IsDead();
}

bool UNFWeaponComponent::GetAimView(FVector& OutLocation, FRotator& OutRotation) const
{
	const APawn* OwnerPawn = GetOwnerPawn();
	if (!OwnerPawn)
	{
		return false;
	}

	if (const AController* Controller = OwnerPawn->GetController())
	{
		Controller->GetPlayerViewPoint(OutLocation, OutRotation);
	}
	else
	{
		OutLocation = OwnerPawn->GetPawnViewLocation();
		OutRotation = OwnerPawn->GetBaseAimRotation();
	}
	return true;
}

int32 UNFWeaponComponent::GetReserveAmmo() const
{
	return GetData()->bInfiniteReserve ? -1 : ReserveAmmo;
}

float UNFWeaponComponent::GetCurrentSpreadDegrees() const
{
	const UNFWeaponData* Data = GetData();

	float Extra = FMath::Min(SpreadHeat * Data->SpreadPerShot, Data->MaxBloom);

	if (const APawn* OwnerPawn = GetOwnerPawn())
	{
		const float Speed = OwnerPawn->GetVelocity().Size2D();
		Extra += Data->MoveSpread * FMath::Clamp(Speed / FMath::Max(Data->MoveSpreadReferenceSpeed, 1.f), 0.f, 1.f);

		const UPawnMovementComponent* Movement = OwnerPawn->GetMovementComponent();
		if (Movement && Movement->IsFalling())
		{
			Extra += Data->AirborneSpread;
		}
	}

	const float Base = FMath::Lerp(Data->HipSpread, Data->AdsSpread, AdsAlpha);
	return (Base + Extra * FMath::Lerp(1.f, Data->AdsSpreadModifier, AdsAlpha)) * EvaluateUse(Data).SpreadScale;
}

float UNFWeaponComponent::GetDesiredFOV(float HipFOV) const
{
	return FMath::Lerp(HipFOV, GetData()->AdsFOV, AdsAlpha);
}

float UNFWeaponComponent::GetLookSensitivityScale(float HipFOV) const
{
	const float HipTan = FMath::Tan(FMath::DegreesToRadians(FMath::Clamp(HipFOV, 10.f, 170.f) * 0.5f));
	const float CurrentTan = FMath::Tan(FMath::DegreesToRadians(FMath::Clamp(GetDesiredFOV(HipFOV), 10.f, 170.f) * 0.5f));
	return (CurrentTan / HipTan) * FMath::Lerp(1.f, GetData()->AdsSensitivityScale, AdsAlpha);
}

FVector UNFWeaponComponent::GetMuzzleLocation() const
{
	const FName MuzzleSocket = GetData()->MuzzleSocketName;
	if (VisualMesh && IsOwnerLocallyControlled() && VisualMesh->DoesSocketExist(MuzzleSocket))
	{
		return VisualMesh->GetSocketLocation(MuzzleSocket);
	}

	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	if (GetAimView(ViewLocation, ViewRotation))
	{
		return ViewLocation + ViewRotation.RotateVector(FallbackMuzzleOffset);
	}
	return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
}

void UNFWeaponComponent::SetAmmo(int32 NewMagazine, int32 NewReserve)
{
	NewMagazine = FMath::Max(NewMagazine, 0);
	NewReserve = FMath::Max(NewReserve, 0);

	const bool bChanged = (NewMagazine != AmmoInMagazine) || (NewReserve != ReserveAmmo);
	AmmoInMagazine = NewMagazine;
	ReserveAmmo = NewReserve;

	if (OwnerHasAuthority())
	{
		RepAmmoInMagazine = AmmoInMagazine;
		RepReserveAmmo = ReserveAmmo;
	}

	if (bChanged)
	{
		OnAmmoChanged.Broadcast(AmmoInMagazine, GetReserveAmmo());
	}
}

// ---------------------------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------------------------

void UNFWeaponComponent::StartFire()
{
	if (!IsOwnerLocallyControlled())
	{
		return;
	}

	bTriggerHeld = true;
	bSemiShotPending = true;
	bDryFiredThisPress = false;
	LastFirePressTime = GetWorld()->GetTimeSeconds();

	// Do not wait for the next tick: trigger -> first shot latency must be zero.
	UpdateFiring(0.f);
}

void UNFWeaponComponent::StopFire()
{
	bTriggerHeld = false;
}

void UNFWeaponComponent::StartReload()
{
	if (!IsOwnerLocallyControlled() || !CanReload())
	{
		return;
	}

	BeginReload();

	if (!OwnerHasAuthority())
	{
		ServerStartReload();
	}
}

void UNFWeaponComponent::CancelReload()
{
	if (!IsOwnerLocallyControlled() || !bReloading)
	{
		return;
	}

	CancelReloadInternal();

	if (!OwnerHasAuthority())
	{
		ServerCancelReload();
	}
}

void UNFWeaponComponent::ServerCancelReload_Implementation()
{
	CancelReloadInternal();
}

void UNFWeaponComponent::SetAiming(bool bNewAiming)
{
	if (!IsOwnerLocallyControlled() || bAiming == bNewAiming)
	{
		return;
	}

	bAiming = bNewAiming;

	if (!OwnerHasAuthority())
	{
		ServerSetAiming(bNewAiming);
	}
}

void UNFWeaponComponent::ServerSetAiming_Implementation(bool bNewAiming)
{
	bAiming = bNewAiming;
}

// ---------------------------------------------------------------------------------------------
// Weapon switching
// ---------------------------------------------------------------------------------------------

void UNFWeaponComponent::SwitchWeapon()
{
	SelectSlot((ActiveSlot + 1) % NumSlots);
}

void UNFWeaponComponent::SelectSlot(int32 Slot)
{
	if (!IsOwnerLocallyControlled() || Slot < 0 || Slot >= NumSlots || Slot == ActiveSlot
		|| bFireBlocked || IsOwnerDead() || IsMeleeActive())
	{
		return;
	}

	// A weapon that cannot be used right now (Gaffer waist deep or with a strongbox) cannot be raised.
	if (!EvaluateUse(GetSlotData(Slot)).bUsable)
	{
		return;
	}

	ApplySlot(Slot);

	if (!OwnerHasAuthority())
	{
		LastSlotRequestTime = GetWorldTime();
		ServerSelectSlot(static_cast<uint8>(Slot));
	}
}

void UNFWeaponComponent::ServerSelectSlot_Implementation(uint8 Slot)
{
	if (Slot >= NumSlots || Slot == ActiveSlot || bFireBlocked || IsOwnerDead())
	{
		return;
	}
	// Refused: the owner keeps its prediction for AmmoReconcileDelay, then falls back to RepActiveSlot.
	if (!EvaluateUse(GetSlotData(Slot), ServerDepthTolerance).bUsable)
	{
		return;
	}
	ApplySlot(Slot);
}

void UNFWeaponComponent::AuthEnsureUsableSlot()
{
	if (!OwnerHasAuthority() || EvaluateUse(GetData()).bUsable)
	{
		return;
	}

	const int32 OtherSlot = (ActiveSlot + 1) % NumSlots;
	if (EvaluateUse(GetSlotData(OtherSlot)).bUsable)
	{
		ApplySlot(OtherSlot);
	}
}

void UNFWeaponComponent::ApplySlot(int32 Slot)
{
	Slot = FMath::Clamp(Slot, 0, NumSlots - 1);
	if (Slot == ActiveSlot)
	{
		return;
	}

	// Switching away loses a running reload (spec: no ammo is given).
	CancelReloadInternal();

	StoredMagazine[ActiveSlot] = AmmoInMagazine;
	StoredReserve[ActiveSlot] = ReserveAmmo;
	ActiveSlot = Slot;

	if (OwnerHasAuthority())
	{
		RepActiveSlot = static_cast<uint8>(Slot);
	}
	else
	{
		// Do not let the reconcile overwrite the new weapon's ammo with the old weapon's replicated count.
		LastPredictedAmmoChangeTime = GetWorldTime();
	}
	SetAmmo(StoredMagazine[Slot], StoredReserve[Slot]);

	SwitchEndTime = GetWorldTime() + FMath::Max(GetData()->EquipTime, 0.f);
	bSemiShotPending = false;
	ShotCooldown = 0.f;
	SpreadHeat = 0.f;
	BurstShotIndex = 0;

	OnWeaponSwitched.Broadcast(ActiveSlot);
	// SetAmmo only broadcasts on a change; the HUD still needs the new weapon's numbers.
	OnAmmoChanged.Broadcast(AmmoInMagazine, GetReserveAmmo());

	if (GetNetMode() != NM_DedicatedServer)
	{
		RequestVisualAssets();
	}
}

void UNFWeaponComponent::OnRep_ActiveSlot()
{
	if (!HasBegunPlay() || ActiveSlot == RepActiveSlot)
	{
		return;
	}

	// The owner predicts its own switches; it only follows the server when it has no request in flight
	// (forced switch on pickup, or a refused request).
	if (IsOwnerLocallyControlled() && GetWorldTime() - LastSlotRequestTime <= AmmoReconcileDelay)
	{
		return;
	}
	ApplySlot(RepActiveSlot);
}

void UNFWeaponComponent::AuthAddReserveAmmo(int32 Slot, int32 Amount)
{
	if (!OwnerHasAuthority() || Slot < 0 || Slot >= NumSlots)
	{
		return;
	}

	const UNFWeaponData* SlotData = GetSlotData(Slot);
	if (SlotData->bInfiniteReserve)
	{
		return;
	}

	if (Slot == ActiveSlot)
	{
		SetAmmo(AmmoInMagazine, Amount < 0 ? FMath::Max(ReserveAmmo, SlotData->StartingReserveAmmo) : ReserveAmmo + Amount);
	}
	else
	{
		StoredReserve[Slot] = Amount < 0 ? FMath::Max(StoredReserve[Slot], SlotData->StartingReserveAmmo) : StoredReserve[Slot] + Amount;
	}
}

// ---------------------------------------------------------------------------------------------
// Firing (owning client / listen host)
// ---------------------------------------------------------------------------------------------

bool UNFWeaponComponent::CanFire() const
{
	return !bFireBlocked && !bReloading && !IsSwitching() && !IsMeleeActive()
		&& AmmoInMagazine > 0 && !IsOwnerDead() && EvaluateUse(GetData()).bUsable;
}

bool UNFWeaponComponent::WantsShot() const
{
	const UNFWeaponData* Data = GetData();
	if (Data->bFullAuto)
	{
		return bTriggerHeld;
	}
	return bSemiShotPending && (GetWorld()->GetTimeSeconds() - LastFirePressTime) <= Data->FireInputBuffer;
}

void UNFWeaponComponent::UpdateFiring(float DeltaTime)
{
	ShotCooldown -= DeltaTime;

	// Frame-rate independent: release every shot that became due this frame and carry the remainder.
	const float Interval = GetData()->GetFireInterval();
	int32 ShotsThisFrame = 0;
	while (ShotCooldown <= 0.f && WantsShot() && ShotsThisFrame < NFWeapon::MaxShotsPerFrame)
	{
		if (!CanFire())
		{
			HandleCannotFire();
			break;
		}

		FireShot();
		bSemiShotPending = false;
		ShotCooldown += Interval;
		++ShotsThisFrame;
	}

	// Idle time is not banked into extra shots.
	if (ShotCooldown < 0.f)
	{
		ShotCooldown = 0.f;
	}
}

void UNFWeaponComponent::HandleCannotFire()
{
	// A press during a weapon switch or a swing stays buffered (FireInputBuffer); everything else drops it.
	if (IsSwitching() || IsMeleeActive())
	{
		return;
	}

	bSemiShotPending = false;

	// Only an empty magazine clicks and reloads; water, strongbox, reload and death are silent.
	if (bFireBlocked || bReloading || AmmoInMagazine > 0 || IsOwnerDead() || !EvaluateUse(GetData()).bUsable)
	{
		return;
	}

	if (!bDryFiredThisPress)
	{
		bDryFiredThisPress = true;
		PlaySoundAtMuzzle(GetData()->DryFireSound);
		OnDryFire.Broadcast();
	}

	if (GetData()->bAutoReloadOnEmpty)
	{
		StartReload();
	}
}

void UNFWeaponComponent::FireShot()
{
	UWorld* World = GetWorld();
	APawn* OwnerPawn = GetOwnerPawn();
	const UNFWeaponData* Data = GetData();

	FVector ViewLocation;
	FRotator ViewRotation;
	if (!World || !OwnerPawn || !GetAimView(ViewLocation, ViewRotation))
	{
		return;
	}

	++ShotCounter;
	FRandomStream Stream(ShotCounter * 7919 + 17);

	// The traces start at the camera so the shot lands where the crosshair is.
	const FVector AimDirection = ViewRotation.Vector();
	const float SpreadRadians = FMath::DegreesToRadians(GetCurrentSpreadDegrees());
	const float RangeCm = Data->RangeMeters * 100.f;
	const FVector Muzzle = GetMuzzleLocation();
	const int32 NumPellets = FMath::Clamp(Data->PelletsPerShot, 1, 16);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(NFWeaponTrace), /*bTraceComplex*/ true, OwnerPawn);

	FNFShotData Shot;
	Shot.Origin = ViewLocation;
	Shot.Direction = AimDirection;
	Shot.ShotId = static_cast<uint8>(ShotCounter & 0xFF);
	Shot.Pellets.Reserve(NumPellets);

	TArray<FVector> TraceEnds;
	TArray<FVector> ImpactNormals;
	uint32 BlockingMask = 0;
	TraceEnds.Reserve(NumPellets);
	ImpactNormals.Reserve(NumPellets);

	for (int32 PelletIndex = 0; PelletIndex < NumPellets; ++PelletIndex)
	{
		// Every pellet is independent inside the cone.
		const FVector PelletDirection = SpreadRadians > KINDA_SMALL_NUMBER ? Stream.VRandCone(AimDirection, SpreadRadians) : AimDirection;
		const FVector MaxRangeEnd = ViewLocation + PelletDirection * RangeCm;

		FHitResult Hit;
		const bool bBlockingHit = World->LineTraceSingleByChannel(Hit, ViewLocation, MaxRangeEnd, NF_ECC_Weapon, QueryParams);

		// Muzzle obstruction: if something sits between the barrel and the camera's impact point, that is what gets hit.
		if (bBlockingHit)
		{
			FHitResult MuzzleHit;
			if (World->LineTraceSingleByChannel(MuzzleHit, Muzzle, Hit.ImpactPoint, NF_ECC_Weapon, QueryParams)
				&& MuzzleHit.GetActor() != Hit.GetActor()
				&& MuzzleHit.Distance < FVector::Dist(Muzzle, Hit.ImpactPoint) - 5.f)
			{
				Hit = MuzzleHit;
			}
		}

		const FVector TraceEnd = bBlockingHit ? FVector(Hit.ImpactPoint) : MaxRangeEnd;
		const FVector ImpactNormal = bBlockingHit ? FVector(Hit.ImpactNormal) : FVector(-PelletDirection);

		FNFPelletData Pellet;
		Pellet.TraceEnd = TraceEnd;
		Pellet.ImpactNormal = ImpactNormal;
		Pellet.HitActor = bBlockingHit ? Hit.GetActor() : nullptr;
		Pellet.HitZone = bBlockingHit ? UNFHealthComponent::ResolveHitZone(Hit) : ENFHitZone::Body;
		Pellet.bBlockingHit = bBlockingHit;
		Shot.Pellets.Add(Pellet);

		TraceEnds.Add(TraceEnd);
		ImpactNormals.Add(ImpactNormal);
		if (bBlockingHit)
		{
			BlockingMask |= (1u << PelletIndex);
		}
	}

	LastShotTime = World->GetTimeSeconds();

	// Local feel first: recoil and cosmetics never wait for the server.
	AddRecoil(Stream);
	SpreadHeat += 1.f;
	PlayFireFX(TraceEnds, ImpactNormals, BlockingMask);
	OnFired.Broadcast(TraceEnds[0]);

	if (OwnerHasAuthority())
	{
		AuthProcessShot(Shot, /*bFromRemoteClient*/ false);
	}
	else
	{
		// Predict the ammo; the server's count is adopted later (see TickComponent).
		LastPredictedAmmoChangeTime = LastShotTime;
		SetAmmo(AmmoInMagazine - 1, ReserveAmmo);
		ServerFire(Shot);
	}
}

// ---------------------------------------------------------------------------------------------
// Firing (server)
// ---------------------------------------------------------------------------------------------

void UNFWeaponComponent::ServerFire_Implementation(const FNFShotData& Shot)
{
	AuthProcessShot(Shot, /*bFromRemoteClient*/ true);
}

void UNFWeaponComponent::AuthProcessShot(const FNFShotData& Shot, bool bFromRemoteClient)
{
	UWorld* World = GetWorld();
	APawn* OwnerPawn = GetOwnerPawn();
	const UNFWeaponData* Data = GetData();
	if (!World || !OwnerPawn || IsOwnerDead() || Shot.Pellets.Num() == 0)
	{
		return;
	}

	if (bFromRemoteClient)
	{
		const float Now = World->GetTimeSeconds();

		if (bFireBlocked || !EvaluateUse(Data, ServerDepthTolerance).bUsable)
		{
			UE_LOG(LogNewFPS, Verbose, TEXT("Shot rejected (%s): weapon blocked (water depth / strongbox / downed)"), *GetNameSafe(OwnerPawn));
			return;
		}

		if (bReloading || AmmoInMagazine <= 0)
		{
			UE_LOG(LogNewFPS, Verbose, TEXT("Shot rejected (%s): reloading=%d ammo=%d"), *GetNameSafe(OwnerPawn), bReloading ? 1 : 0, AmmoInMagazine);
			return;
		}

		// The client started these timers one latency before the server did.
		if (Now < SwitchEndTime - ServerReloadLeniency || Now < MeleeEndTime - ServerReloadLeniency)
		{
			UE_LOG(LogNewFPS, Verbose, TEXT("Shot rejected (%s): switching weapon or swinging"), *GetNameSafe(OwnerPawn));
			return;
		}

		// Fire-rate check as a token bucket: exact on average, tolerant of bunched packets.
		if (Now < ServerNextFireTime - ServerFireRateTolerance)
		{
			UE_LOG(LogNewFPS, Verbose, TEXT("Shot rejected (%s): fire rate"), *GetNameSafe(OwnerPawn));
			return;
		}
		ServerNextFireTime = FMath::Max(ServerNextFireTime, Now - ServerFireRateTolerance) + Data->GetFireInterval();
	}

	// One trigger pull costs one round, however many pellets it carries.
	SetAmmo(AmmoInMagazine - 1, ReserveAmmo);

	const FVector Origin = Shot.Origin;

	// The shot must start near where the server believes the shooter's eyes are; if not, no pellet counts.
	bool bOriginValid = true;
	if (bFromRemoteClient && FVector::DistSquared(Origin, OwnerPawn->GetPawnViewLocation()) > FMath::Square(ServerMaxOriginError))
	{
		UE_LOG(LogNewFPS, Verbose, TEXT("Hits rejected (%s): origin too far from pawn"), *GetNameSafe(OwnerPawn));
		bOriginValid = false;
	}

	// A client cannot send more pellets than the weapon fires.
	const int32 NumPellets = FMath::Min(Shot.Pellets.Num(), FMath::Clamp(Data->PelletsPerShot, 1, 16));

	// Pellets that hit the same target are merged into one hit, so the target sees the total of the shot.
	struct FTargetTotal
	{
		AActor* Actor = nullptr;
		float Damage = 0.f;
		int32 Pellets = 0;
		bool bHead = false;
		FVector HitLocation = FVector::ZeroVector;
	};
	TArray<FTargetTotal, TInlineAllocator<4>> Totals;

	TArray<FVector_NetQuantize> TraceEnds;
	TraceEnds.Reserve(NumPellets);
	uint8 BlockingMask = 0;

	for (int32 PelletIndex = 0; PelletIndex < NumPellets; ++PelletIndex)
	{
		const FNFPelletData& Pellet = Shot.Pellets[PelletIndex];
		TraceEnds.Add(Pellet.TraceEnd);
		if (Pellet.bBlockingHit && PelletIndex < 8)
		{
			BlockingMask |= static_cast<uint8>(1 << PelletIndex);
		}

		AActor* HitActor = Pellet.HitActor;
		if (!bOriginValid || !Pellet.bBlockingHit || !HitActor || HitActor == OwnerPawn || IsFriendlyTarget(HitActor))
		{
			continue;
		}
		if (bFromRemoteClient && !AuthValidatePellet(Shot, Pellet, OwnerPawn, Data))
		{
			continue;
		}
		if (!UNFHealthComponent::FindHealthComponent(HitActor))
		{
			continue;
		}

		const FVector TraceEnd = Pellet.TraceEnd;
		const float PelletDamage = Data->Damage
			* Data->GetDistanceMultiplier(FVector::Dist(Origin, TraceEnd))
			* Data->GetZoneMultiplier(Pellet.HitZone);

		FTargetTotal* Total = Totals.FindByPredicate([HitActor](const FTargetTotal& Entry) { return Entry.Actor == HitActor; });
		if (!Total)
		{
			Total = &Totals.AddDefaulted_GetRef();
			Total->Actor = HitActor;
			Total->HitLocation = TraceEnd;
		}
		Total->Damage += PelletDamage;
		Total->Pellets += 1;
		Total->bHead |= (Pellet.HitZone == ENFHitZone::Head);
	}

	for (const FTargetTotal& Total : Totals)
	{
		UNFHealthComponent* Health = UNFHealthComponent::FindHealthComponent(Total.Actor);
		if (!Health)
		{
			continue;
		}

		// Distance and zone are already inside the per-pellet sum; the health component adds resistance.
		FNFDamageInfo DamageInfo;
		DamageInfo.BaseDamage = Total.Damage;
		DamageInfo.DistanceMultiplier = 1.f;
		DamageInfo.ZoneMultiplier = 1.f;
		DamageInfo.Zone = Total.bHead ? ENFHitZone::Head : ENFHitZone::Body;
		DamageInfo.DamageType = ENFDamageType::Bullet;
		DamageInfo.HitLocation = Total.HitLocation;
		DamageInfo.ShotDirection = (Total.HitLocation - Origin).GetSafeNormal();
		DamageInfo.Instigator = OwnerPawn->GetController();
		DamageInfo.Causer = OwnerPawn;

		UE_LOG(LogNewFPS, Verbose, TEXT("Shot (%s -> %s): %d pellet(s), total %.1f before resistance"),
			*GetNameSafe(OwnerPawn), *GetNameSafe(Total.Actor), Total.Pellets, Total.Damage);

		const float AppliedDamage = Health->ApplyDamage(DamageInfo);
		if (AppliedDamage > 0.f)
		{
			ClientConfirmHit(Total.Actor, AppliedDamage, DamageInfo.Zone, Health->IsDead());
		}
	}

	MulticastFireFX(TraceEnds, BlockingMask);
}

bool UNFWeaponComponent::AuthValidatePellet(const FNFShotData& Shot, const FNFPelletData& Pellet, const APawn* OwnerPawn, const UNFWeaponData* Data) const
{
	const FVector Origin = Shot.Origin;
	const FVector TraceEnd = Pellet.TraceEnd;
	const AActor* HitActor = Pellet.HitActor;
	const float Distance = FVector::Dist(Origin, TraceEnd);

	// 1. Range.
	if (Distance > Data->RangeMeters * 100.f * 1.05f)
	{
		UE_LOG(LogNewFPS, Verbose, TEXT("Hit rejected (%s): out of range"), *GetNameSafe(OwnerPawn));
		return false;
	}

	// 2. The pellet must lie inside the widest cone this weapon can produce around the reported aim direction.
	if (Distance > NFWeapon::ConeCheckMinDistance)
	{
		const float MaxSpreadDegrees = (Data->HipSpread + Data->MoveSpread + Data->AirborneSpread + Data->MaxBloom)
			* GetTideRules()->WaistSpreadMultiplier + ServerSpreadTolerance;
		const FVector AimDirection = FVector(Shot.Direction).GetSafeNormal();
		const float CosAngle = FVector::DotProduct((TraceEnd - Origin) / Distance, AimDirection);
		if (CosAngle < FMath::Cos(FMath::DegreesToRadians(FMath::Min(MaxSpreadDegrees, 89.f))))
		{
			UE_LOG(LogNewFPS, Verbose, TEXT("Hit rejected (%s): outside the spread cone"), *GetNameSafe(OwnerPawn));
			return false;
		}
	}

	// 3. The impact must be on (or near, to cover latency) the claimed actor as the server sees it now.
	const FBox TargetBounds = HitActor->GetComponentsBoundingBox(/*bNonColliding*/ false);
	if (!TargetBounds.IsValid || !TargetBounds.ExpandBy(ServerHitBoundsTolerance).IsInsideOrOn(TraceEnd))
	{
		UE_LOG(LogNewFPS, Verbose, TEXT("Hit rejected (%s): impact not on %s"), *GetNameSafe(OwnerPawn), *GetNameSafe(HitActor));
		return false;
	}

	// 4. Line of sight against static world geometry (no shooting through walls).
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(NFWeaponServerLOS), /*bTraceComplex*/ true);
	QueryParams.AddIgnoredActor(OwnerPawn);
	QueryParams.AddIgnoredActor(HitActor);
	const FVector LOSEnd = TraceEnd - (TraceEnd - Origin).GetSafeNormal() * 10.f;
	if (GetWorld()->LineTraceTestByObjectType(Origin, LOSEnd, FCollisionObjectQueryParams(ECC_WorldStatic), QueryParams))
	{
		UE_LOG(LogNewFPS, Verbose, TEXT("Hit rejected (%s): no line of sight"), *GetNameSafe(OwnerPawn));
		return false;
	}

	return true;
}

void UNFWeaponComponent::ClientConfirmHit_Implementation(AActor* HitActor, float Damage, ENFHitZone Zone, bool bKilled)
{
	const UNFWeaponData* Data = GetData();
	const TSoftObjectPtr<USoundBase>& ConfirmSound = bKilled ? Data->KillConfirmSound
		: (Zone == ENFHitZone::Head ? Data->HeadshotConfirmSound : Data->HitConfirmSound);
	if (USoundBase* Sound = ConfirmSound.Get())
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}

	OnHitConfirmed.Broadcast(HitActor, Damage, Zone, bKilled);
}

void UNFWeaponComponent::MulticastFireFX_Implementation(const TArray<FVector_NetQuantize>& TraceEnds, uint8 BlockingMask)
{
	// The shooter already played its cosmetics at fire time.
	if (GetNetMode() == NM_DedicatedServer || IsOwnerLocallyControlled() || TraceEnds.Num() == 0)
	{
		return;
	}

	TArray<FVector> Ends;
	Ends.Reserve(TraceEnds.Num());
	for (const FVector_NetQuantize& TraceEnd : TraceEnds)
	{
		Ends.Add(TraceEnd);
	}
	// Impact normals are not sent; PlayFireFX faces the impact effect back along the shot.
	PlayFireFX(Ends, TArray<FVector>(), BlockingMask);
}

// ---------------------------------------------------------------------------------------------
// Melee (Kedge)
// ---------------------------------------------------------------------------------------------

bool UNFWeaponComponent::CanMelee(float DepthSlack) const
{
	// Works up to chest depth (it is the only tool there), not under water, not with a strongbox under the arm.
	return !bFireBlocked && !bCarrying && !IsOwnerDead()
		&& GetOwnerWaterDepth() < GetTideRules()->OverheadDepth + DepthSlack;
}

void UNFWeaponComponent::StartMelee()
{
	if (!IsOwnerLocallyControlled() || IsMeleeActive() || !CanMelee(0.f))
	{
		return;
	}

	BeginMeleeLocal();

	if (OwnerHasAuthority())
	{
		AuthStartMelee();
	}
	else
	{
		ServerStartMelee();
	}
}

void UNFWeaponComponent::BeginMeleeLocal()
{
	// The swinging hand is the reloading hand: the reload is lost (spec 4.3), and it does not restart by itself.
	CancelReloadInternal();

	MeleeEndTime = GetWorldTime() + FMath::Max(GetMeleeData()->SwingInterval, 0.05f);
	bSemiShotPending = false;

	if (GetNetMode() != NM_DedicatedServer)
	{
		PlaySoundAtMuzzle(GetMeleeData()->SwingSound);
	}
	OnMeleeStarted.Broadcast();
}

void UNFWeaponComponent::ServerStartMelee_Implementation()
{
	const float Now = GetWorldTime();
	if (!CanMelee(ServerDepthTolerance) || Now < ServerNextMeleeTime - ServerFireRateTolerance)
	{
		UE_LOG(LogNewFPS, Verbose, TEXT("Melee rejected (%s)"), *GetNameSafe(GetOwner()));
		return;
	}
	ServerNextMeleeTime = FMath::Max(ServerNextMeleeTime, Now - ServerFireRateTolerance) + GetMeleeData()->SwingInterval;

	BeginMeleeLocal();
	AuthStartMelee();
}

void UNFWeaponComponent::AuthStartMelee()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float HitDelay = GetMeleeData()->HitDelay;
	if (HitDelay <= 0.f)
	{
		AuthMeleeHit();
	}
	else
	{
		World->GetTimerManager().SetTimer(MeleeHitTimerHandle, this, &UNFWeaponComponent::AuthMeleeHit, HitDelay, false);
	}

	MulticastMeleeFX();
}

void UNFWeaponComponent::MulticastMeleeFX_Implementation()
{
	// The swinger and the server already started their swing.
	if (OwnerHasAuthority() || IsOwnerLocallyControlled())
	{
		return;
	}
	BeginMeleeLocal();
}

void UNFWeaponComponent::AuthMeleeHit()
{
	UWorld* World = GetWorld();
	APawn* OwnerPawn = GetOwnerPawn();
	if (!World || !OwnerPawn || IsOwnerDead() || bCarrying || bFireBlocked)
	{
		return;
	}

	// Server authoritative: the target is found here, from the server's own idea of the swinger's view.
	const UNFMeleeData* Melee = GetMeleeData();
	const FVector Eye = OwnerPawn->GetPawnViewLocation();
	const FVector ViewDirection = OwnerPawn->GetBaseAimRotation().Vector();
	const float Reach = Melee->Reach;
	const float MinCos = FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(Melee->ConeAngle * 0.5f, 0.5f, 90.f)));

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(NFMeleeOverlap), /*bTraceComplex*/ false, OwnerPawn);
	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByChannel(Overlaps, Eye, FQuat::Identity, NF_ECC_Weapon, FCollisionShape::MakeSphere(Reach), QueryParams);

	// First contact only: the nearest damageable target inside the cone.
	AActor* BestActor = nullptr;
	FVector BestPoint = FVector::ZeroVector;
	float BestDistance = Reach;
	TArray<AActor*, TInlineAllocator<8>> Checked;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Candidate = Overlap.GetActor();
		if (!Candidate || Candidate == OwnerPawn || Checked.Contains(Candidate))
		{
			continue;
		}
		Checked.Add(Candidate);

		const UNFHealthComponent* CandidateHealth = UNFHealthComponent::FindHealthComponent(Candidate);
		if (!CandidateHealth || CandidateHealth->IsDead() || IsFriendlyTarget(Candidate))
		{
			continue;
		}

		const FBox Bounds = Candidate->GetComponentsBoundingBox(/*bNonColliding*/ false);
		if (!Bounds.IsValid)
		{
			continue;
		}

		const FVector ClosestPoint = Bounds.GetClosestPointTo(Eye);
		const float Distance = FVector::Dist(Eye, ClosestPoint);
		if (Distance > BestDistance)
		{
			continue;
		}

		// A target the eye is practically inside of is always in the cone.
		const FVector ToTarget = Bounds.GetCenter() - Eye;
		if (Distance > 30.f && FVector::DotProduct(ToTarget.GetSafeNormal(), ViewDirection) < MinCos)
		{
			continue;
		}

		FCollisionQueryParams LOSParams(SCENE_QUERY_STAT(NFMeleeLOS), /*bTraceComplex*/ true);
		LOSParams.AddIgnoredActor(OwnerPawn);
		LOSParams.AddIgnoredActor(Candidate);
		if (World->LineTraceTestByObjectType(Eye, ClosestPoint, FCollisionObjectQueryParams(ECC_WorldStatic), LOSParams))
		{
			continue;
		}

		BestActor = Candidate;
		BestPoint = ClosestPoint;
		BestDistance = Distance;
	}

	if (!BestActor)
	{
		return;
	}

	UNFHealthComponent* Health = UNFHealthComponent::FindHealthComponent(BestActor);
	if (!Health)
	{
		return;
	}

	// No weak-point multiplier and no falloff for the Kedge.
	FNFDamageInfo DamageInfo;
	DamageInfo.BaseDamage = Melee->Damage;
	DamageInfo.Zone = ENFHitZone::Body;
	DamageInfo.DamageType = ENFDamageType::Melee;
	DamageInfo.HitLocation = BestPoint;
	DamageInfo.ShotDirection = (BestActor->GetActorLocation() - OwnerPawn->GetActorLocation()).GetSafeNormal2D();
	DamageInfo.Instigator = OwnerPawn->GetController();
	DamageInfo.Causer = OwnerPawn;

	const float AppliedDamage = Health->ApplyDamage(DamageInfo);
	if (AppliedDamage > 0.f)
	{
		ClientConfirmHit(BestActor, AppliedDamage, ENFHitZone::Body, Health->IsDead());
	}
}

// ---------------------------------------------------------------------------------------------
// Reload
// ---------------------------------------------------------------------------------------------

bool UNFWeaponComponent::CanReload() const
{
	const UNFWeaponData* Data = GetData();
	return !bReloading
		&& !bFireBlocked
		&& !IsSwitching()
		&& !IsMeleeActive()
		&& AmmoInMagazine < Data->MagazineSize
		&& (Data->bInfiniteReserve || ReserveAmmo > 0)
		&& !IsOwnerDead()
		&& EvaluateUse(Data, OwnerHasAuthority() && !IsOwnerLocallyControlled() ? ServerDepthTolerance : 0.f).bCanReload;
}

void UNFWeaponComponent::ServerStartReload_Implementation()
{
	if (CanReload())
	{
		BeginReload();
	}
}

void UNFWeaponComponent::BeginReload()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const UNFWeaponData* Data = GetData();
	bReloading = true;

	// Waist deep: one hand keeps the weapon out of the water (spec: reload time x1.25).
	float Duration = Data->ReloadTime * EvaluateUse(Data).ReloadTimeScale;
	if (OwnerHasAuthority() && !IsOwnerLocallyControlled())
	{
		// The remote client started its timer one latency earlier than we did.
		Duration = FMath::Max(Duration - ServerReloadLeniency, 0.05f);
	}
	World->GetTimerManager().SetTimer(ReloadTimerHandle, this, &UNFWeaponComponent::FinishReload, Duration, false);

	PlaySoundAtMuzzle(Data->ReloadSound);
	OnReloadStateChanged.Broadcast(true);
}

void UNFWeaponComponent::FinishReload()
{
	const UNFWeaponData* Data = GetData();
	bReloading = false;

	const int32 Missing = FMath::Max(Data->MagazineSize - AmmoInMagazine, 0);
	const int32 Loaded = Data->bInfiniteReserve ? Missing : FMath::Min(Missing, ReserveAmmo);

	if (!OwnerHasAuthority())
	{
		LastPredictedAmmoChangeTime = GetWorld()->GetTimeSeconds();
	}
	SetAmmo(AmmoInMagazine + Loaded, Data->bInfiniteReserve ? ReserveAmmo : ReserveAmmo - Loaded);

	OnReloadStateChanged.Broadcast(false);
}

void UNFWeaponComponent::CancelReloadInternal()
{
	if (!bReloading)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReloadTimerHandle);
	}
	// Progress is lost and no ammo is given.
	bReloading = false;
	OnReloadStateChanged.Broadcast(false);
}

void UNFWeaponComponent::OnRep_Reloading()
{
	// Non-owning clients only (COND_SkipOwner).
	if (bReloading)
	{
		PlaySoundAtMuzzle(GetData()->ReloadSound);
	}
	OnReloadStateChanged.Broadcast(bReloading);
}

// ---------------------------------------------------------------------------------------------
// Recoil (owning client)
// ---------------------------------------------------------------------------------------------

void UNFWeaponComponent::AddRecoil(FRandomStream& Stream)
{
	const UNFWeaponData* Data = GetData();
	FVector2D Kick = Data->GetRecoilForShot(BurstShotIndex);
	Kick.X += Stream.FRandRange(-Data->RecoilRandomness, Data->RecoilRandomness);
	Kick.Y += Stream.FRandRange(-Data->RecoilRandomness, Data->RecoilRandomness);
	Kick *= FMath::Lerp(1.f, Data->AdsRecoilMultiplier, AdsAlpha);

	PendingRecoil += Kick;
	++BurstShotIndex;
}

void UNFWeaponComponent::UpdateRecoil(float DeltaTime)
{
	const UNFWeaponData* Data = GetData();
	const float TimeSinceShot = GetWorld()->GetTimeSeconds() - LastShotTime;

	if (TimeSinceShot > Data->RecoilResetTime)
	{
		BurstShotIndex = 0;
	}

	if (!PendingRecoil.IsNearlyZero(1.e-4))
	{
		// Feed the kick into the camera quickly but not in a single frame.
		const float Alpha = FMath::Clamp(Data->RecoilKickSpeed * DeltaTime, 0.f, 1.f);
		const FVector2D Step = PendingRecoil.SizeSquared() < 1.e-4 ? PendingRecoil : PendingRecoil * Alpha;
		PendingRecoil -= Step;
		RecoverableRecoil += Step;
		ApplyViewDelta(Step.X, Step.Y);
	}
	else if (TimeSinceShot > Data->RecoilRecoveryDelay && Data->RecoilRecoverySpeed > 0.f && !RecoverableRecoil.IsNearlyZero(1.e-4))
	{
		const double Remaining = RecoverableRecoil.Size();
		const double StepSize = FMath::Min<double>(Data->RecoilRecoverySpeed * DeltaTime, Remaining);
		const FVector2D Step = RecoverableRecoil * (StepSize / Remaining);
		RecoverableRecoil -= Step;
		ApplyViewDelta(-Step.X, -Step.Y);
	}
	else if (RecoverableRecoil.IsNearlyZero(1.e-4))
	{
		RecoverableRecoil = FVector2D::ZeroVector;
	}
}

void UNFWeaponComponent::NotifyAimInput(float YawDelta, float PitchDelta)
{
	// Player input that opposes the accumulated kick has already "paid" for it.
	if (RecoverableRecoil.Y > 0.0 && PitchDelta < 0.f)
	{
		RecoverableRecoil.Y = FMath::Max(0.0, RecoverableRecoil.Y + PitchDelta);
	}
	else if (RecoverableRecoil.Y < 0.0 && PitchDelta > 0.f)
	{
		RecoverableRecoil.Y = FMath::Min(0.0, RecoverableRecoil.Y + PitchDelta);
	}

	if (RecoverableRecoil.X > 0.0 && YawDelta < 0.f)
	{
		RecoverableRecoil.X = FMath::Max(0.0, RecoverableRecoil.X + YawDelta);
	}
	else if (RecoverableRecoil.X < 0.0 && YawDelta > 0.f)
	{
		RecoverableRecoil.X = FMath::Min(0.0, RecoverableRecoil.X + YawDelta);
	}
}

void UNFWeaponComponent::ApplyViewDelta(float YawDelta, float PitchDelta) const
{
	const APawn* OwnerPawn = GetOwnerPawn();
	APlayerController* PlayerController = OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
	if (!PlayerController)
	{
		return;
	}

	FRotator ControlRotation = PlayerController->GetControlRotation();
	ControlRotation.Yaw += YawDelta;
	ControlRotation.Pitch = FMath::ClampAngle(ControlRotation.Pitch + PitchDelta, -NFWeapon::MaxViewPitch, NFWeapon::MaxViewPitch);
	PlayerController->SetControlRotation(ControlRotation);
}

// ---------------------------------------------------------------------------------------------
// Cosmetics
// ---------------------------------------------------------------------------------------------

void UNFWeaponComponent::PlayFireFX(const TArray<FVector>& TraceEnds, const TArray<FVector>& ImpactNormals, uint32 BlockingMask)
{
	UWorld* World = GetWorld();
	if (!World || GetNetMode() == NM_DedicatedServer || TraceEnds.Num() == 0)
	{
		return;
	}

	const UNFWeaponData* Data = GetData();
	const FVector Muzzle = GetMuzzleLocation();
	const bool bDebugDraw = CVarNFDebugShots.GetValueOnGameThread() > 0;

	// One report and one flash per trigger pull; one tracer and one impact per pellet.
	PlaySoundAtMuzzle(Data->FireSound);

	if (UNiagaraSystem* MuzzleFlash = Data->MuzzleFlashFX.Get())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, MuzzleFlash, Muzzle, (TraceEnds[0] - Muzzle).Rotation());
	}

	UNiagaraSystem* Tracer = Data->TracerFX.Get();
	UNiagaraSystem* Impact = Data->ImpactFX.Get();

	for (int32 Index = 0; Index < TraceEnds.Num(); ++Index)
	{
		const FVector& TraceEnd = TraceEnds[Index];
		const bool bBlockingHit = Index < 32 && (BlockingMask & (1u << Index)) != 0;

		// The tracer leaves the barrel and converges on the camera's impact point.
		if (Tracer)
		{
			if (UNiagaraComponent* TracerComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Tracer, Muzzle, (TraceEnd - Muzzle).Rotation()))
			{
				TracerComponent->SetVectorParameter(Data->TracerEndParameter, TraceEnd);
			}
		}

		if (bBlockingHit && Impact)
		{
			const FVector Normal = ImpactNormals.IsValidIndex(Index) ? ImpactNormals[Index] : (Muzzle - TraceEnd).GetSafeNormal();
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Impact, TraceEnd, Normal.Rotation());
		}

		if (bDebugDraw)
		{
			DrawDebugLine(World, Muzzle, TraceEnd, FColor(255, 200, 80), false, 0.08f, 0, 0.6f);
			if (bBlockingHit)
			{
				DrawDebugPoint(World, TraceEnd, 6.f, FColor::Orange, false, 1.5f);
			}
		}
	}
}

void UNFWeaponComponent::PlaySoundAtMuzzle(const TSoftObjectPtr<USoundBase>& Sound) const
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	if (USoundBase* LoadedSound = Sound.Get())
	{
		UGameplayStatics::PlaySoundAtLocation(this, LoadedSound, GetMuzzleLocation());
	}
}

void UNFWeaponComponent::RequestVisualAssets()
{
	const UNFWeaponData* Data = GetData();

	TArray<FSoftObjectPath> Paths;
	auto AddPath = [&Paths](const FSoftObjectPath& Path)
	{
		if (!Path.IsNull())
		{
			Paths.AddUnique(Path);
		}
	};

	AddPath(Data->WeaponSkeletalMesh.ToSoftObjectPath());
	AddPath(Data->WeaponStaticMesh.ToSoftObjectPath());
	AddPath(Data->FireSound.ToSoftObjectPath());
	AddPath(Data->DryFireSound.ToSoftObjectPath());
	AddPath(Data->ReloadSound.ToSoftObjectPath());
	AddPath(Data->HitConfirmSound.ToSoftObjectPath());
	AddPath(Data->HeadshotConfirmSound.ToSoftObjectPath());
	AddPath(Data->KillConfirmSound.ToSoftObjectPath());
	AddPath(Data->MuzzleFlashFX.ToSoftObjectPath());
	AddPath(Data->TracerFX.ToSoftObjectPath());
	AddPath(Data->ImpactFX.ToSoftObjectPath());

	if (Paths.Num() == 0)
	{
		HandleVisualAssetsLoaded();
		return;
	}

	// The handle keeps the loaded assets alive for as long as this weapon exists.
	VisualAssetsHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		MoveTemp(Paths),
		FStreamableDelegate::CreateUObject(this, &UNFWeaponComponent::HandleVisualAssetsLoaded));
}

void UNFWeaponComponent::HandleVisualAssetsLoaded()
{
	OnVisualsReady.Broadcast();
}
