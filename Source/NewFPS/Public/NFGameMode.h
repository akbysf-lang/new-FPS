#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NFGameMode.generated.h"

class AController;

/** Prototype rules: default classes and a timed respawn for dead players. */
UCLASS()
class NEWFPS_API ANFGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANFGameMode();

	/** Called by the dead player's character on the server. */
	void NotifyPlayerDied(AController* Victim, AController* Killer);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS", meta = (ClampMin = "0.1"))
	float PlayerRespawnDelay = 3.f;

private:
	void RespawnPlayer(TWeakObjectPtr<AController> Controller);
};
