#include "NFGameMode.h"
#include "NewFPS.h"
#include "NFCharacter.h"
#include "NFHUD.h"
#include "NFPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

ANFGameMode::ANFGameMode()
{
	DefaultPawnClass = ANFCharacter::StaticClass();
	PlayerControllerClass = ANFPlayerController::StaticClass();
	HUDClass = ANFHUD::StaticClass();
}

void ANFGameMode::NotifyPlayerDied(AController* Victim, AController* Killer)
{
	if (!Victim)
	{
		return;
	}

	UE_LOG(LogNewFPS, Log, TEXT("%s was killed by %s"), *GetNameSafe(Victim), *GetNameSafe(Killer));

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateUObject(this, &ANFGameMode::RespawnPlayer, TWeakObjectPtr<AController>(Victim)),
		PlayerRespawnDelay,
		false);
}

void ANFGameMode::RespawnPlayer(TWeakObjectPtr<AController> Controller)
{
	AController* Victim = Controller.Get();
	if (!Victim)
	{
		return;
	}

	if (APawn* OldPawn = Victim->GetPawn())
	{
		Victim->UnPossess();
		OldPawn->Destroy();
	}

	RestartPlayer(Victim);
}
