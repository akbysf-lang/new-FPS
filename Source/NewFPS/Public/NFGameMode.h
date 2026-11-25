#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NFTypes.h"
#include "NFGameMode.generated.h"

class AController;
class ANFCharacter;
class UNFRulesData;

/**
 * Prototype mission rules (prototype-spec.md 6 and 8.3), server only:
 *  - quota = players + 1 (max 6), kept on ANFGameState;
 *  - downed players: bleed out -> lifeline respawn; solo: one spare line, the second fall fails the mission;
 *  - everyone down or out at once fails the mission; the end of the tide table ends it (success = quota met);
 *  - Flood Drill debug start: console "nf.FloodDrill", URL option "?FloodDrill", or bStartInFloodDrill.
 */
UCLASS()
class NEWFPS_API ANFGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANFGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	/** Never null: falls back to the UNFRulesData class defaults. */
	const UNFRulesData* GetRules() const;

	/** Called by the character on the server when it drops to 0 health. */
	void NotifyPlayerDowned(ANFCharacter* Character);

	/** Called by the character on the server when it bled out. */
	void NotifyPlayerOut(ANFCharacter* Character);

	/**
	 * Debug start for tuning (spec 8.3): tide clock to 4:30, players to the player starts tagged "FloodDrill"
	 * (put them at loot point C), 2 Attendants + 4 Scuttlers around them, quota = 1 strongbox per 2 players.
	 */
	UFUNCTION(BlueprintCallable, Category = "NewFPS")
	void StartFloodDrill();

	/** Reload the map (non-seamless travel to "?Restart"). */
	UFUNCTION(BlueprintCallable, Category = "NewFPS")
	void RestartMission();

	/** PlayerStartTag of the player starts the Flood Drill moves the players to. */
	static const FName FloodDrillStartTag;

protected:
	virtual void BeginPlay() override;

	/** Mission and player rules. Leave empty to use the class defaults of UNFRulesData. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Rules")
	TObjectPtr<UNFRulesData> RulesData;

	/** Start every play session in the Flood Drill (same as the "?FloodDrill" URL option). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Rules")
	bool bStartInFloodDrill = false;

	/** Reload the map this long after the mission ended. 0 = stay on the result (restart with "nf.Mission.Restart"). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "NewFPS|Rules", meta = (ClampMin = "0.0", Units = "s"))
	float RestartDelayAfterMission = 0.f;

private:
	void UpdateQuota();
	void CheckMissionEnd();
	void CheckEveryoneDown();
	void EndMission(ENFMissionResult Result);
	void ApplySpareLine(TWeakObjectPtr<ANFCharacter> Character);
	void RespawnPlayer(TWeakObjectPtr<AController> Controller);

	bool bFloodDrillActive = false;
	bool bSpareLineUsed = false;

	FTimerHandle MissionCheckTimerHandle;
	FTimerHandle RestartTimerHandle;
};
