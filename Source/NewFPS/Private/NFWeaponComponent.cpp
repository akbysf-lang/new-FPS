#include "NFWeaponComponent.h"
#include "NewFPS.h"
#include "NFHealthComponent.h"
#include "NFWeaponData.h"
#include "Components/MeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/AssetManager.h"
#include "Engine/HitResult.h"
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
}

void UNFWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	const UNFWeaponData* Data = GetData();
	SetAmmo(Data->MagazineSize, Data->bInfiniteReserve ? 0 : Data->StartingReserveAmmo);

	// Clients start from the same guess so the reconcile in Tick does not zero the ammo before the first replication.
	RepAmmoInMagazine = AmmoInMagazine;
	RepReserveAmmo = ReserveAmmo;

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

	UpdateFiring(DeltaTime);
	UpdateRecoil(DeltaTime);

	// Adopt the server's ammo once our predictions have had time to be acknowledged.
	if (!OwnerHasAuthority() && !bReloading)
	{
		const float Now = GetWorld()->GetTimeSeconds();
		if (Now - LastPredictedAmmoChangeTime > AmmoReconcileDelay
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
	return WeaponData ? WeaponData.Get() : GetDefault<UNFWeaponData>();
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
	return Base + Extra * FMath::Lerp(1.f, Data->AdsSpreadModifier, AdsAlpha);
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
// Firing (owning client / listen host)
// ---------------------------------------------------------------------------------------------

bool UNFWeaponComponent::CanFire() const
{
	return !bReloading && AmmoInMagazine > 0 && !IsOwnerDead();
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
	bSemiShotPending = false;

	if (bReloading || AmmoInMagazine > 0 || IsOwnerDead())
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

	// The trace starts at the camera so the shot lands where the crosshair is.
	const FVector AimDirection = ViewRotation.Vector();
	const float SpreadRadians = FMath::DegreesToRadians(GetCurrentSpreadDegrees());
	const FVector ShotDirection = SpreadRadians > KINDA_SMALL_NUMBER ? Stream.VRandCone(AimDirection, SpreadRadians) : AimDirection;
	const FVector MaxRangeEnd = ViewLocation + ShotDirection * (Data->RangeMeters * 100.f);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(NFWeaponTrace), /*bTraceComplex*/ true, OwnerPawn);

	FHitResult Hit;
	bool bBlockingHit = World->LineTraceSingleByChannel(Hit, ViewLocation, MaxRangeEnd, NF_ECC_Weapon, QueryParams);

	// Muzzle obstruction: if something sits between the barrel and the camera's impact point, that is what gets hit.
	if (bBlockingHit)
	{
		const FVector Muzzle = GetMuzzleLocation();
		FHitResult MuzzleHit;
		if (World->LineTraceSingleByChannel(MuzzleHit, Muzzle, Hit.ImpactPoint, NF_ECC_Weapon, QueryParams)
			&& MuzzleHit.GetActor() != Hit.GetActor()
			&& MuzzleHit.Distance < FVector::Dist(Muzzle, Hit.ImpactPoint) - 5.f)
		{
			Hit = MuzzleHit;
		}
	}

	FNFShotData Shot;
	Shot.Origin = ViewLocation;
	Shot.Direction = ShotDirection;
	Shot.TraceEnd = bBlockingHit ? FVector(Hit.ImpactPoint) : MaxRangeEnd;
	Shot.ImpactNormal = bBlockingHit ? FVector(Hit.ImpactNormal) : FVector(-ShotDirection);
	Shot.HitActor = bBlockingHit ? Hit.GetActor() : nullptr;
	Shot.HitZone = bBlockingHit ? UNFHealthComponent::ResolveHitZone(Hit) : ENFHitZone::Body;
	Shot.bBlockingHit = bBlockingHit;
	Shot.ShotId = static_cast<uint8>(ShotCounter & 0xFF);

	LastShotTime = World->GetTimeSeconds();

	// Local feel first: recoil and cosmetics never wait for the server.
	AddRecoil(Stream);
	SpreadHeat += 1.f;
	PlayFireFX(Shot.TraceEnd, Shot.ImpactNormal, Shot.bBlockingHit);
	OnFired.Broadcast(Shot.TraceEnd);

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
	if (!World || !OwnerPawn || IsOwnerDead())
	{
		return;
	}

	if (bFromRemoteClient)
	{
		if (bReloading || AmmoInMagazine <= 0)
		{
			UE_LOG(LogNewFPS, Verbose, TEXT("Shot rejected (%s): reloading=%d ammo=%d"), *GetNameSafe(OwnerPawn), bReloading ? 1 : 0, AmmoInMagazine);
			return;
		}

		// Fire-rate check as a token bucket: exact on average, tolerant of bunched packets.
		const float Now = World->GetTimeSeconds();
		if (Now < ServerNextFireTime - ServerFireRateTolerance)
		{
			UE_LOG(LogNewFPS, Verbose, TEXT("Shot rejected (%s): fire rate"), *GetNameSafe(OwnerPawn));
			return;
		}
		ServerNextFireTime = FMath::Max(ServerNextFireTime, Now - ServerFireRateTolerance) + Data->GetFireInterval();
	}

	SetAmmo(AmmoInMagazine - 1, ReserveAmmo);

	AActor* HitActor = Shot.HitActor;
	const bool bHitClaimValid = Shot.bBlockingHit && HitActor && HitActor != OwnerPawn
		&& (!bFromRemoteClient || AuthValidateHitClaim(Shot, OwnerPawn, Data));

	if (bHitClaimValid)
	{
		if (UNFHealthComponent* Health = UNFHealthComponent::FindHealthComponent(HitActor))
		{
			const FVector Origin = Shot.Origin;
			const FVector TraceEnd = Shot.TraceEnd;

			FNFDamageInfo DamageInfo;
			DamageInfo.BaseDamage = Data->Damage;
			DamageInfo.DistanceMultiplier = Data->GetDistanceMultiplier(FVector::Dist(Origin, TraceEnd));
			DamageInfo.Zone = Shot.HitZone;
			DamageInfo.ZoneMultiplier = Data->GetZoneMultiplier(Shot.HitZone);
			DamageInfo.HitLocation = TraceEnd;
			DamageInfo.ShotDirection = (TraceEnd - Origin).GetSafeNormal();
			DamageInfo.Instigator = OwnerPawn->GetController();
			DamageInfo.Causer = OwnerPawn;

			const float AppliedDamage = Health->ApplyDamage(DamageInfo);
			if (AppliedDamage > 0.f)
			{
				ClientConfirmHit(HitActor, AppliedDamage, Shot.HitZone, Health->IsDead());
			}
		}
	}

	MulticastFireFX(Shot.TraceEnd, Shot.ImpactNormal, Shot.bBlockingHit);
}

bool UNFWeaponComponent::AuthValidateHitClaim(const FNFShotData& Shot, const APawn* OwnerPawn, const UNFWeaponData* Data) const
{
	const FVector Origin = Shot.Origin;
	const FVector TraceEnd = Shot.TraceEnd;
	const AActor* HitActor = Shot.HitActor;

	// 1. The shot must start near where the server believes the shooter's eyes are.
	if (FVector::DistSquared(Origin, OwnerPawn->GetPawnViewLocation()) > FMath::Square(ServerMaxOriginError))
	{
		UE_LOG(LogNewFPS, Verbose, TEXT("Hit rejected (%s): origin too far from pawn"), *GetNameSafe(OwnerPawn));
		return false;
	}

	// 2. Range.
	const float MaxRange = Data->RangeMeters * 100.f * 1.05f;
	if (FVector::DistSquared(Origin, TraceEnd) > FMath::Square(MaxRange))
	{
		UE_LOG(LogNewFPS, Verbose, TEXT("Hit rejected (%s): out of range"), *GetNameSafe(OwnerPawn));
		return false;
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

void UNFWeaponComponent::MulticastFireFX_Implementation(const FVector_NetQuantize& TraceEnd, const FVector_NetQuantizeNormal& ImpactNormal, bool bBlockingHit)
{
	// The shooter already played its cosmetics at fire time.
	if (GetNetMode() == NM_DedicatedServer || IsOwnerLocallyControlled())
	{
		return;
	}
	PlayFireFX(TraceEnd, ImpactNormal, bBlockingHit);
}

// ---------------------------------------------------------------------------------------------
// Reload
// ---------------------------------------------------------------------------------------------

bool UNFWeaponComponent::CanReload() const
{
	const UNFWeaponData* Data = GetData();
	return !bReloading
		&& AmmoInMagazine < Data->MagazineSize
		&& (Data->bInfiniteReserve || ReserveAmmo > 0)
		&& !IsOwnerDead();
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

	float Duration = Data->ReloadTime;
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

void UNFWeaponComponent::PlayFireFX(const FVector& TraceEnd, const FVector& ImpactNormal, bool bBlockingHit)
{
	UWorld* World = GetWorld();
	if (!World || GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const UNFWeaponData* Data = GetData();
	const FVector Muzzle = GetMuzzleLocation();
	const FRotator MuzzleRotation = (TraceEnd - Muzzle).Rotation();

	PlaySoundAtMuzzle(Data->FireSound);

	if (UNiagaraSystem* MuzzleFlash = Data->MuzzleFlashFX.Get())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, MuzzleFlash, Muzzle, MuzzleRotation);
	}

	// The tracer leaves the barrel and converges on the camera's impact point.
	if (UNiagaraSystem* Tracer = Data->TracerFX.Get())
	{
		if (UNiagaraComponent* TracerComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Tracer, Muzzle, MuzzleRotation))
		{
			TracerComponent->SetVectorParameter(Data->TracerEndParameter, TraceEnd);
		}
	}

	if (bBlockingHit)
	{
		if (UNiagaraSystem* Impact = Data->ImpactFX.Get())
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, Impact, TraceEnd, ImpactNormal.Rotation());
		}
	}

	if (CVarNFDebugShots.GetValueOnGameThread() > 0)
	{
		DrawDebugLine(World, Muzzle, TraceEnd, FColor(255, 200, 80), false, 0.08f, 0, 0.6f);
		if (bBlockingHit)
		{
			DrawDebugPoint(World, TraceEnd, 6.f, FColor::Orange, false, 1.5f);
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
