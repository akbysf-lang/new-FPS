#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "NFTideData.h"
#include "NFTypes.h"
#include "NFTideManager.generated.h"

class USceneComponent;
class UStaticMeshComponent;

/**
 * The only tide state that travels over the network. Everything else (phase, height, wetness, speed penalty)
 * is computed locally from this and the synchronised server clock.
 */
USTRUCT()
struct FNFTideClock
{
	GENERATED_BODY()

	/** Server world time at which the tide clock read zero. Only meaningful while bRunning. */
	UPROPERTY()
	double CycleStartServerTime = 0.0;

	/** Tide clock reading while paused (or before the tide was started). */
	UPROPERTY()
	double FrozenElapsed = 0.0;

	UPROPERTY()
	bool bRunning = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNFOnTidePhaseChanged, ENFTidePhase, NewPhase, ENFTidePhase, OldPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNFOnTideWarning, FName, WarningId, ENFTidePhase, UpcomingPhase, float, SecondsUntil);

/**
 * The mission tide clock. Place exactly one in a level that has a tide.
 *
 * Network model: the server replicates FNFTideClock once (and again only when the tide is paused, resumed or
 * moved). Every machine derives the water height from AGameStateBase::GetServerWorldTimeSeconds and the tide
 * table, so nothing is sent per frame and late joiners are correct as soon as the clock arrives.
 */
UCLASS()
class NEWFPS_API ANFTideManager : public AActor
{
	GENERATED_BODY()

public:
	ANFTideManager();

	/** The level's tide manager, or null when the level has no tide. */
	static ANFTideManager* Get(const UWorld* World);

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ---- Queries (valid on server and clients) ----

	/** Never null: falls back to the UNFTideData class defaults when no asset is assigned. */
	UFUNCTION(BlueprintPure, Category = "Tide")
	const UNFTideData* GetTideData() const;

	/** Seconds on the tide clock. */
	double GetElapsedSeconds() const;

	UFUNCTION(BlueprintPure, Category = "Tide")
	FNFTideSample GetCurrentSample() const;

	/** World Z of the global water plane right now. Cached per frame. */
	UFUNCTION(BlueprintPure, Category = "Tide")
	float GetGlobalWaterZ() const;

	/** World Z of the water at a location: the global plane, lowered by any local cap (closed sluice gates). */
	UFUNCTION(BlueprintPure, Category = "Tide")
	float GetWaterZAt(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintPure, Category = "Tide")
	bool IsTideRunning() const { return TideClock.bRunning; }

	/** Short English label for HUD/debug. */
	static const TCHAR* GetPhaseLabel(ENFTidePhase Phase);

	// ---- Control (server only) ----

	/** Start (or restart) the tide with the clock at InitialElapsedSeconds. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tide")
	void StartTide(float InitialElapsedSeconds = 0.f);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tide")
	void SetTidePaused(bool bPaused);

	/** Jump the tide clock (scripted surges, debugging). */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Tide")
	void SetTideElapsed(float NewElapsedSeconds);

	// ---- Events (fire locally on every machine; bind bells/announcements here) ----

	/** A new phase began. Also fires once when the tide first becomes known (OldPhase == NewPhase). */
	UPROPERTY(BlueprintAssignable, Category = "Tide")
	FNFOnTidePhaseChanged OnTidePhaseChanged;

	/** A UNFTideData::Warnings entry came due. */
	UPROPERTY(BlueprintAssignable, Category = "Tide")
	FNFOnTideWarning OnTideWarning;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnRep_TideClock();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	/**
	 * Optional visual water surface; only its Z follows the tide. Defaults to the engine plane (100 cm, scaled up).
	 * Assign another Static Mesh / material on the component, or clear the mesh to have no visual.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> WaterPlane;

	/** Tide table and wetness rules. Leave empty to use the class defaults of UNFTideData. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tide")
	TObjectPtr<UNFTideData> TideData;

	/** The server starts the tide in BeginPlay. Turn off to start it from mission script (StartTide). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tide")
	bool bAutoStart = true;

	/** Tide clock reading at start; also where the editor preview puts the water plane. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tide", meta = (ClampMin = "0.0", Units = "s"))
	float StartElapsedSeconds = 0.f;

	/** Added to the data asset's water heights, so one tide table can serve levels built at different heights. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tide", meta = (Units = "cm"))
	float WaterZOffset = 0.f;

	/** How often phase changes and warnings are checked. Not the water update rate: height is computed on demand. */
	UPROPERTY(EditAnywhere, Category = "Tide", meta = (ClampMin = "0.05", Units = "s"))
	float EventPollInterval = 0.2f;

private:
	double GetServerTimeSeconds() const;
	void SetTideClock(const FNFTideClock& NewClock);
	void HandleTideClockChanged();
	void PollTideEvents();
	void UpdateWaterPlane(float WaterZ);

	UPROPERTY(ReplicatedUsing = OnRep_TideClock)
	FNFTideClock TideClock;

	mutable uint64 CachedWaterFrame = MAX_uint64;
	mutable float CachedGlobalWaterZ = 0.f;

	bool bHasLastSample = false;
	bool bResyncWarnings = false;
	int64 LastPhaseSerial = 0;
	ENFTidePhase LastPhase = ENFTidePhase::LowSlack;
	float LastPhaseRemaining = 0.f;

	FTimerHandle EventTimerHandle;
};

/** Per-world registry: O(1) access to the tide manager and the list of regional water modifiers. */
UCLASS()
class NEWFPS_API UNFTideSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	ANFTideManager* GetTideManager() const;
	void RegisterTideManager(ANFTideManager* Manager);
	void UnregisterTideManager(ANFTideManager* Manager);

	/** Modifier must implement INFLocalWaterLevel. */
	void RegisterLocalWaterModifier(UObject* Modifier);
	void UnregisterLocalWaterModifier(UObject* Modifier);

	/** min(GlobalWaterZ, every registered cap that applies at WorldLocation). */
	float ApplyLocalWaterCaps(const FVector& WorldLocation, float GlobalWaterZ) const;

private:
	TWeakObjectPtr<ANFTideManager> TideManager;
	TArray<TWeakObjectPtr<UObject>> LocalWaterModifiers;
};
