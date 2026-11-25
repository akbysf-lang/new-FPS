#include "NFGameMode.h"
#include "NewFPS.h"
#include "NFCharacter.h"
#include "NFEnemyDirector.h"
#include "NFEnemySpawner.h"
#include "NFGameState.h"
#include "NFHealthComponent.h"
#include "NFHUD.h"
#include "NFPlayerController.h"
#include "NFRulesData.h"
#include "NFTideData.h"
#include "NFTideManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

const FName ANFGameMode::FloodDrillStartTag(TEXT("FloodDrill"));

namespace NFGameModeConsole
{
	ANFGameMode* FindGameMode(UWorld* World)
	{
		ANFGameMode* GameMode = World ? World->GetAuthGameMode<ANFGameMode>() : nullptr;
		if (!GameMode)
		{
			UE_LOG(LogNewFPS, Warning, TEXT("nf: run this on the server/host of a level that uses ANFGameMode."));
		}
		return GameMode;
	}
}

static FAutoConsoleCommandWithWorldAndArgs GNFFloodDrillCommand(
	TEXT("nf.FloodDrill"),
	TEXT("Server only. Debug start: tide clock to 4:30, players to the 'FloodDrill' player starts, enemies ready, small quota."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (ANFGameMode* GameMode = NFGameModeConsole::FindGameMode(World))
		{
			GameMode->StartFloodDrill();
		}
	}));

static FAutoConsoleCommandWithWorldAndArgs GNFMissionRestartCommand(
	TEXT("nf.Mission.Restart"),
	TEXT("Server only. Reload the map and start the mission again."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (ANFGameMode* GameMode = NFGameModeConsole::FindGameMode(World))
		{
			GameMode->RestartMission();
		}
	}));

ANFGameMode::ANFGameMode()
{
	DefaultPawnClass = ANFCharacter::StaticClass();
	PlayerControllerClass = ANFPlayerController::StaticClass();
	HUDClass = ANFHUD::StaticClass();
	GameStateClass = ANFGameState::StaticClass();
}

const UNFRulesData* ANFGameMode::GetRules() const
{
	return RulesData ? RulesData.Get() : GetDefault<UNFRulesData>();
}

void ANFGameMode::BeginPlay()
{
	Super::BeginPlay();

	UpdateQuota();
	GetWorldTimerManager().SetTimer(MissionCheckTimerHandle, this, &ANFGameMode::CheckMissionEnd, 1.f, true);

	if (bStartInFloodDrill || UGameplayStatics::HasOption(OptionsString, TEXT("FloodDrill")))
	{
		// One second in: the first player has a pawn by then.
		FTimerHandle DrillTimerHandle;
		GetWorldTimerManager().SetTimer(DrillTimerHandle, this, &ANFGameMode::StartFloodDrill, 1.f, false);
	}
}

void ANFGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	UpdateQuota();
}

void ANFGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	UpdateQuota();
}

void ANFGameMode::UpdateQuota()
{
	ANFGameState* NFGameState = GetGameState<ANFGameState>();
	if (!NFGameState)
	{
		return;
	}

	const UNFRulesData* Rules = GetRules();
	const int32 NumPlayers = FMath::Max(GetNumPlayers(), 1);
	const int32 Quota = bFloodDrillActive
		? FMath::Max(NumPlayers / FMath::Max(Rules->FloodDrillPlayersPerStrongbox, 1), 1)
		: FMath::Clamp(NumPlayers + Rules->QuotaPlayersPlus, 1, FMath::Max(Rules->MaxQuota, 1));
	NFGameState->AuthSetQuota(Quota);
}

// ---------------------------------------------------------------------------------------------
// Downed / out / mission end
// ---------------------------------------------------------------------------------------------

void ANFGameMode::NotifyPlayerDowned(ANFCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	UE_LOG(LogNewFPS, Log, TEXT("%s is down"), *GetNameSafe(Character));

	// Solo (spec 6.3): one spare line per mission, the second fall ends it.
	if (GetNumPlayers() <= 1)
	{
		if (!bSpareLineUsed)
		{
			bSpareLineUsed = true;
			FTimerHandle SpareLineTimerHandle;
			GetWorldTimerManager().SetTimer(
				SpareLineTimerHandle,
				FTimerDelegate::CreateUObject(this, &ANFGameMode::ApplySpareLine, TWeakObjectPtr<ANFCharacter>(Character)),
				FMath::Max(GetRules()->SpareLineDelay, 0.1f),
				false);
		}
		else
		{
			EndMission(ENFMissionResult::Failed);
		}
		return;
	}

	CheckEveryoneDown();
}

void ANFGameMode::ApplySpareLine(TWeakObjectPtr<ANFCharacter> Character)
{
	ANFCharacter* Player = Character.Get();
	if (!Player)
	{
		return;
	}

	const UNFRulesData* Rules = GetRules();
	if (Player->AuthReviveFromDowned(Rules->SpareLineHealth, Rules->SpareLineInvulnerableSeconds))
	{
		if (UNFEnemyDirector* Director = GetWorld()->GetSubsystem<UNFEnemyDirector>())
		{
			Director->KnockbackEnemiesAround(Player->GetActorLocation(), Rules->SpareLineKnockbackRadius, Rules->SpareLineKnockbackDistance);
		}
	}
}

void ANFGameMode::NotifyPlayerOut(ANFCharacter* Character)
{
	if (!Character)
	{
		return;
	}

	UE_LOG(LogNewFPS, Log, TEXT("%s bled out; the lifeline brings them back in %.0f s"), *GetNameSafe(Character), GetRules()->LifelineRespawnDelay);

	CheckEveryoneDown();

	if (AController* Controller = Character->GetController())
	{
		FTimerHandle RespawnTimerHandle;
		GetWorldTimerManager().SetTimer(
			RespawnTimerHandle,
			FTimerDelegate::CreateUObject(this, &ANFGameMode::RespawnPlayer, TWeakObjectPtr<AController>(Controller)),
			FMath::Max(GetRules()->LifelineRespawnDelay, 0.1f),
			false);
	}
}

void ANFGameMode::RespawnPlayer(TWeakObjectPtr<AController> Controller)
{
	AController* Victim = Controller.Get();
	const ANFGameState* NFGameState = GetGameState<ANFGameState>();
	if (!Victim || (NFGameState && NFGameState->GetMissionResult() != ENFMissionResult::InProgress))
	{
		return;
	}

	if (APawn* OldPawn = Victim->GetPawn())
	{
		Victim->UnPossess();
		OldPawn->Destroy();
	}

	// Back down the line onto a player start (the winch platform in the spec's map).
	RestartPlayer(Victim);

	if (const ANFCharacter* NewCharacter = Cast<ANFCharacter>(Victim->GetPawn()))
	{
		if (UNFHealthComponent* Health = NewCharacter->GetHealthComponent())
		{
			Health->ReviveTo(GetRules()->LifelineRespawnHealth);
		}
	}
}

void ANFGameMode::CheckEveryoneDown()
{
	int32 NumPlayerPawns = 0;
	int32 NumStanding = 0;
	for (TActorIterator<ANFCharacter> It(GetWorld()); It; ++It)
	{
		if (!It->IsPlayerControlled())
		{
			continue;
		}
		++NumPlayerPawns;
		if (It->GetLifeState() == ENFLifeState::Alive)
		{
			++NumStanding;
		}
	}

	if (NumPlayerPawns > 0 && NumStanding == 0)
	{
		EndMission(ENFMissionResult::Failed);
	}
}

void ANFGameMode::CheckMissionEnd()
{
	// The shift ends with the tide table (12:00 in the spec). A looping table never ends the mission.
	const ANFTideManager* TideManager = ANFTideManager::Get(GetWorld());
	if (!TideManager || !TideManager->IsTideRunning())
	{
		return;
	}

	const UNFTideData* TideData = TideManager->GetTideData();
	if (TideData->bLoop || TideManager->GetElapsedSeconds() < TideData->CycleDurationSeconds)
	{
		return;
	}

	const ANFGameState* NFGameState = GetGameState<ANFGameState>();
	EndMission(NFGameState && NFGameState->IsQuotaMet() ? ENFMissionResult::Success : ENFMissionResult::Failed);
}

void ANFGameMode::EndMission(ENFMissionResult Result)
{
	ANFGameState* NFGameState = GetGameState<ANFGameState>();
	if (!NFGameState || NFGameState->GetMissionResult() != ENFMissionResult::InProgress)
	{
		return;
	}

	UE_LOG(LogNewFPS, Log, TEXT("Mission ended: %s (%d / %d strongboxes)"),
		Result == ENFMissionResult::Success ? TEXT("success") : TEXT("failed"),
		NFGameState->GetDeliveredStrongboxes(), NFGameState->GetQuota());

	NFGameState->AuthSetMissionResult(Result);

	for (TActorIterator<ANFEnemySpawner> It(GetWorld()); It; ++It)
	{
		It->SetSpawningEnabled(false);
	}

	if (RestartDelayAfterMission > 0.f)
	{
		GetWorldTimerManager().SetTimer(RestartTimerHandle, this, &ANFGameMode::RestartMission, RestartDelayAfterMission, false);
	}
}

void ANFGameMode::RestartMission()
{
	if (UWorld* World = GetWorld())
	{
		World->ServerTravel(TEXT("?Restart"), false);
	}
}

// ---------------------------------------------------------------------------------------------
// Flood Drill (spec 8.3)
// ---------------------------------------------------------------------------------------------

void ANFGameMode::StartFloodDrill()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const UNFRulesData* Rules = GetRules();
	UE_LOG(LogNewFPS, Log, TEXT("Flood Drill: tide clock to %.0f s"), Rules->FloodDrillStartSeconds);

	bFloodDrillActive = true;
	UpdateQuota();

	if (ANFTideManager* TideManager = ANFTideManager::Get(World))
	{
		TideManager->StartTide(Rules->FloodDrillStartSeconds);
	}
	else
	{
		UE_LOG(LogNewFPS, Warning, TEXT("Flood Drill: this level has no ANFTideManager; only the players and enemies are set up."));
	}

	// Players go to the player starts tagged FloodDrill (place them at loot point C).
	TArray<APlayerStart*> DrillStarts;
	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		if (It->PlayerStartTag == FloodDrillStartTag)
		{
			DrillStarts.Add(*It);
		}
	}
	if (DrillStarts.Num() == 0)
	{
		UE_LOG(LogNewFPS, Warning, TEXT("Flood Drill: no Player Start with PlayerStartTag '%s'; players stay where they are."), *FloodDrillStartTag.ToString());
	}

	FVector DrillCenter = FVector::ZeroVector;
	bool bHasCenter = false;
	int32 PlayerIndex = 0;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();
		APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
		if (!Pawn)
		{
			continue;
		}

		if (DrillStarts.Num() > 0)
		{
			const APlayerStart* Start = DrillStarts[PlayerIndex % DrillStarts.Num()];
			// More players than starts: line the extra ones up beside the start.
			const FVector Offset = Start->GetActorRightVector() * (100.f * static_cast<float>(PlayerIndex / DrillStarts.Num()));
			Pawn->TeleportTo(Start->GetActorLocation() + Offset, Start->GetActorRotation());
			PlayerController->SetControlRotation(Start->GetActorRotation());
		}

		if (!bHasCenter)
		{
			DrillCenter = Pawn->GetActorLocation();
			bHasCenter = true;
		}
		++PlayerIndex;
	}

	// The wave script continues from the drill's start time; the drill's own enemies stand ready.
	for (TActorIterator<ANFEnemySpawner> It(World); It; ++It)
	{
		It->AuthResetToTime(Rules->FloodDrillStartSeconds);
		if (bHasCenter)
		{
			It->AuthSpawnGroupAround(ENFEnemyFaction::Wardens, Rules->FloodDrillAttendants, DrillCenter, Rules->FloodDrillEnemyMinDistance, Rules->FloodDrillEnemyMaxDistance);
			It->AuthSpawnGroupAround(ENFEnemyFaction::Drift, Rules->FloodDrillScuttlers, DrillCenter, Rules->FloodDrillEnemyMinDistance, Rules->FloodDrillEnemyMaxDistance);
		}
		break;
	}
}
