#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NFTypes.h"
#include "NFGameState.generated.h"

/** Replicated mission scoreboard: strongboxes on the winch net, the quota, and how the shift ended. */
UCLASS()
class NEWFPS_API ANFGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Mission")
	int32 GetDeliveredStrongboxes() const { return DeliveredStrongboxes; }

	UFUNCTION(BlueprintPure, Category = "Mission")
	int32 GetQuota() const { return Quota; }

	UFUNCTION(BlueprintPure, Category = "Mission")
	bool IsQuotaMet() const { return DeliveredStrongboxes >= Quota; }

	UFUNCTION(BlueprintPure, Category = "Mission")
	ENFMissionResult GetMissionResult() const { return MissionResult; }

	// ---- Server only ----

	void AuthAddDeliveredStrongbox();
	void AuthSetQuota(int32 NewQuota);
	void AuthSetMissionResult(ENFMissionResult NewResult);

private:
	UPROPERTY(Replicated)
	int32 DeliveredStrongboxes = 0;

	UPROPERTY(Replicated)
	int32 Quota = 2;

	UPROPERTY(Replicated)
	ENFMissionResult MissionResult = ENFMissionResult::InProgress;
};
