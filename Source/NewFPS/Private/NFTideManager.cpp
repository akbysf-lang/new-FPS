#include "NFTideManager.h"
#include "NewFPS.h"
#include "NFLocalWaterLevel.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/IConsoleManager.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

namespace NFTide
{
	ANFTideManager* FindAuthorityManager(UWorld* World)
	{
		ANFTideManager* Manager = ANFTideManager::Get(World);
		if (!Manager)
		{
			UE_LOG(LogNewFPS, Warning, TEXT("nf.Tide: this level has no ANFTideManager."));
			return nullptr;
		}
		if (!Manager->HasAuthority())
		{
			UE_LOG(LogNewFPS, Warning, TEXT("nf.Tide: run this on the server/host; clients cannot move the tide."));
			return nullptr;
		}
		return Manager;
	}
}

static FAutoConsoleCommandWithWorldAndArgs GNFTideSetElapsedCommand(
	TEXT("nf.Tide.SetElapsed"),
	TEXT("Server only. Jump the tide clock to <seconds>."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (Args.Num() < 1)
		{
			return;
		}
		if (ANFTideManager* Manager = NFTide::FindAuthorityManager(World))
		{
			Manager->SetTideElapsed(FCString::Atof(*Args[0]));
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs GNFTidePauseCommand(
	TEXT("nf.Tide.Pause"),
	TEXT("Server only. 1 = freeze the tide, 0 = let it run."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (Args.Num() < 1)
		{
			return;
		}
		if (ANFTideManager* Manager = NFTide::FindAuthorityManager(World))
		{
			Manager->SetTidePaused(FCString::Atoi(*Args[0]) != 0);
		}
	}));

// ---------------------------------------------------------------------------------------------
// ANFTideManager
// ---------------------------------------------------------------------------------------------

ANFTideManager::ANFTideManager()
{
	// Tick only moves the visual plane; it is switched off where there is nothing to draw.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	bReplicates = true;
	bAlwaysRelevant = true;
	// The clock changes a handful of times per mission; changes are pushed with ForceNetUpdate.
	SetNetUpdateFrequency(2.f);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	WaterPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterPlane"));
	WaterPlane->SetupAttachment(SceneRoot);
	WaterPlane->SetMobility(EComponentMobility::Movable);
	WaterPlane->SetRelativeScale3D(FVector(1000.f, 1000.f, 1.f));
	WaterPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WaterPlane->SetGenerateOverlapEvents(false);
	WaterPlane->SetCanEverAffectNavigation(false);
	WaterPlane->SetCastShadow(false);
	if (PlaneMesh.Succeeded())
	{
		WaterPlane->SetStaticMesh(PlaneMesh.Object);
	}
}

void ANFTideManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANFTideManager, TideClock);
}

ANFTideManager* ANFTideManager::Get(const UWorld* World)
{
	const UNFTideSubsystem* Subsystem = World ? World->GetSubsystem<UNFTideSubsystem>() : nullptr;
	return Subsystem ? Subsystem->GetTideManager() : nullptr;
}

const TCHAR* ANFTideManager::GetPhaseLabel(ENFTidePhase Phase)
{
	switch (Phase)
	{
	case ENFTidePhase::LowSlack:
		return TEXT("SLACK LOW");
	case ENFTidePhase::Flood:
		return TEXT("FLOOD");
	case ENFTidePhase::HighSlack:
		return TEXT("SLACK HIGH");
	case ENFTidePhase::Ebb:
		return TEXT("EBB");
	default:
		return TEXT("TIDE");
	}
}

void ANFTideManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Editor preview: show the water where it will be when the mission starts.
	UpdateWaterPlane(GetTideData()->Evaluate(StartElapsedSeconds).WaterZ + WaterZOffset);
}

void ANFTideManager::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// Registered before any BeginPlay so characters find the tide on their first movement update.
	if (UWorld* World = GetWorld())
	{
		if (UNFTideSubsystem* Subsystem = World->GetSubsystem<UNFTideSubsystem>())
		{
			Subsystem->RegisterTideManager(this);
		}
	}
}

void ANFTideManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && bAutoStart)
	{
		StartTide(StartElapsedSeconds);
	}

	const bool bHasVisual = GetNetMode() != NM_DedicatedServer && WaterPlane && WaterPlane->GetStaticMesh() != nullptr;
	SetActorTickEnabled(bHasVisual);

	GetWorldTimerManager().SetTimer(EventTimerHandle, this, &ANFTideManager::PollTideEvents, FMath::Max(EventPollInterval, 0.05f), true);
}

void ANFTideManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(EventTimerHandle);

	if (UWorld* World = GetWorld())
	{
		if (UNFTideSubsystem* Subsystem = World->GetSubsystem<UNFTideSubsystem>())
		{
			Subsystem->UnregisterTideManager(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ANFTideManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateWaterPlane(GetGlobalWaterZ());
}

// ---- Queries ----

const UNFTideData* ANFTideManager::GetTideData() const
{
	return TideData ? TideData.Get() : GetDefault<UNFTideData>();
}

double ANFTideManager::GetServerTimeSeconds() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0;
	}
	// On the server this is the world time; on clients it is the replicated server time estimate.
	if (const AGameStateBase* GameState = World->GetGameState())
	{
		return GameState->GetServerWorldTimeSeconds();
	}
	return World->GetTimeSeconds();
}

double ANFTideManager::GetElapsedSeconds() const
{
	if (!TideClock.bRunning)
	{
		return TideClock.FrozenElapsed;
	}
	return FMath::Max(GetServerTimeSeconds() - TideClock.CycleStartServerTime, 0.0);
}

FNFTideSample ANFTideManager::GetCurrentSample() const
{
	FNFTideSample Sample = GetTideData()->Evaluate(GetElapsedSeconds());
	Sample.WaterZ += WaterZOffset;
	return Sample;
}

float ANFTideManager::GetGlobalWaterZ() const
{
	// Movement asks several times per character per frame; the answer cannot change within a frame.
	if (CachedWaterFrame != GFrameCounter)
	{
		CachedWaterFrame = GFrameCounter;
		CachedGlobalWaterZ = GetCurrentSample().WaterZ;
	}
	return CachedGlobalWaterZ;
}

float ANFTideManager::GetWaterZAt(const FVector& WorldLocation) const
{
	const float GlobalWaterZ = GetGlobalWaterZ();
	const UWorld* World = GetWorld();
	const UNFTideSubsystem* Subsystem = World ? World->GetSubsystem<UNFTideSubsystem>() : nullptr;
	return Subsystem ? Subsystem->ApplyLocalWaterCaps(WorldLocation, GlobalWaterZ) : GlobalWaterZ;
}

// ---- Control ----

void ANFTideManager::StartTide(float InitialElapsedSeconds)
{
	if (!HasAuthority())
	{
		return;
	}

	FNFTideClock NewClock;
	NewClock.bRunning = true;
	NewClock.FrozenElapsed = FMath::Max(InitialElapsedSeconds, 0.f);
	NewClock.CycleStartServerTime = GetServerTimeSeconds() - NewClock.FrozenElapsed;
	SetTideClock(NewClock);
}

void ANFTideManager::SetTidePaused(bool bPaused)
{
	if (!HasAuthority() || bPaused == !TideClock.bRunning)
	{
		return;
	}

	FNFTideClock NewClock = TideClock;
	if (bPaused)
	{
		NewClock.FrozenElapsed = GetElapsedSeconds();
		NewClock.bRunning = false;
	}
	else
	{
		NewClock.CycleStartServerTime = GetServerTimeSeconds() - TideClock.FrozenElapsed;
		NewClock.bRunning = true;
	}
	SetTideClock(NewClock);
}

void ANFTideManager::SetTideElapsed(float NewElapsedSeconds)
{
	if (!HasAuthority())
	{
		return;
	}

	FNFTideClock NewClock = TideClock;
	NewClock.FrozenElapsed = FMath::Max(NewElapsedSeconds, 0.f);
	NewClock.CycleStartServerTime = GetServerTimeSeconds() - NewClock.FrozenElapsed;
	SetTideClock(NewClock);
}

void ANFTideManager::SetTideClock(const FNFTideClock& NewClock)
{
	TideClock = NewClock;
	ForceNetUpdate();
	HandleTideClockChanged();
}

void ANFTideManager::OnRep_TideClock()
{
	HandleTideClockChanged();
}

void ANFTideManager::HandleTideClockChanged()
{
	CachedWaterFrame = MAX_uint64;
	// A clock jump must not replay warnings that belong to the time that was skipped.
	bResyncWarnings = true;
	PollTideEvents();
	UpdateWaterPlane(GetGlobalWaterZ());
}

// ---- Events ----

void ANFTideManager::PollTideEvents()
{
	const UWorld* World = GetWorld();
	if (!World || !HasActorBegunPlay())
	{
		return;
	}
	// Clients have no server clock until the GameState has replicated.
	if (!HasAuthority() && World->GetGameState() == nullptr)
	{
		return;
	}

	const UNFTideData* Data = GetTideData();
	const FNFTideSample Sample = GetCurrentSample();
	if (!Sample.bValid)
	{
		return;
	}

	const int64 PhaseSerial = static_cast<int64>(Sample.CycleIndex) * Data->Phases.Num() + Sample.PhaseIndex;
	if (!bHasLastSample || PhaseSerial != LastPhaseSerial)
	{
		const bool bFirstSample = !bHasLastSample;
		const ENFTidePhase OldPhase = bFirstSample ? Sample.Phase : LastPhase;

		bHasLastSample = true;
		LastPhaseSerial = PhaseSerial;
		LastPhase = Sample.Phase;
		// Joining mid-phase: warnings that were already due stay silent.
		if (bFirstSample)
		{
			LastPhaseRemaining = Sample.LabelRemaining;
		}

		UE_LOG(LogNewFPS, Log, TEXT("Tide phase: %s (%.0f s left, water Z %.1f)"), GetPhaseLabel(Sample.Phase), Sample.PhaseRemaining, Sample.WaterZ);
		OnTidePhaseChanged.Broadcast(Sample.Phase, OldPhase);
	}

	if (bResyncWarnings)
	{
		bResyncWarnings = false;
		LastPhaseRemaining = Sample.LabelRemaining;
	}

	// Warnings count down to the next LABEL change (the tide turning), not to the next table entry: the spec's
	// table has several entries per label (F1..F5 are all "Flood"). When the label changes, LabelRemaining jumps
	// up, so nothing fires by accident.
	for (const FNFTideWarning& Warning : Data->Warnings)
	{
		if (Warning.UpcomingPhase == Sample.NextLabel
			&& Sample.NextLabel != Sample.Phase
			&& LastPhaseRemaining > Warning.SecondsBefore
			&& Sample.LabelRemaining <= Warning.SecondsBefore)
		{
			OnTideWarning.Broadcast(Warning.WarningId, Warning.UpcomingPhase, Sample.LabelRemaining);
		}
	}
	LastPhaseRemaining = Sample.LabelRemaining;
}

void ANFTideManager::UpdateWaterPlane(float WaterZ)
{
	if (!WaterPlane)
	{
		return;
	}
	FVector PlaneLocation = WaterPlane->GetComponentLocation();
	if (!FMath::IsNearlyEqual(static_cast<float>(PlaneLocation.Z), WaterZ, 0.01f))
	{
		PlaneLocation.Z = WaterZ;
		WaterPlane->SetWorldLocation(PlaneLocation);
	}
}

// ---------------------------------------------------------------------------------------------
// UNFTideSubsystem
// ---------------------------------------------------------------------------------------------

ANFTideManager* UNFTideSubsystem::GetTideManager() const
{
	return TideManager.Get();
}

void UNFTideSubsystem::RegisterTideManager(ANFTideManager* Manager)
{
	if (TideManager.IsValid() && TideManager.Get() != Manager)
	{
		UE_LOG(LogNewFPS, Warning, TEXT("More than one ANFTideManager in the level; %s is ignored."), *GetNameSafe(Manager));
		return;
	}
	TideManager = Manager;
}

void UNFTideSubsystem::UnregisterTideManager(ANFTideManager* Manager)
{
	if (TideManager.Get() == Manager)
	{
		TideManager.Reset();
	}
}

void UNFTideSubsystem::RegisterLocalWaterModifier(UObject* Modifier)
{
	if (Modifier && Cast<INFLocalWaterLevel>(Modifier) != nullptr)
	{
		LocalWaterModifiers.AddUnique(TWeakObjectPtr<UObject>(Modifier));
	}
}

void UNFTideSubsystem::UnregisterLocalWaterModifier(UObject* Modifier)
{
	LocalWaterModifiers.Remove(TWeakObjectPtr<UObject>(Modifier));
}

float UNFTideSubsystem::ApplyLocalWaterCaps(const FVector& WorldLocation, float GlobalWaterZ) const
{
	float WaterZ = GlobalWaterZ;
	for (const TWeakObjectPtr<UObject>& WeakModifier : LocalWaterModifiers)
	{
		const INFLocalWaterLevel* Modifier = Cast<INFLocalWaterLevel>(WeakModifier.Get());
		float MaxWaterZ = 0.f;
		if (Modifier && Modifier->GetLocalWaterCap(WorldLocation, MaxWaterZ))
		{
			WaterZ = FMath::Min(WaterZ, MaxWaterZ);
		}
	}
	return WaterZ;
}
