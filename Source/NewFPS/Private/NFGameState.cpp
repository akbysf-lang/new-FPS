#include "NFGameState.h"
#include "NewFPS.h"
#include "Net/UnrealNetwork.h"

void ANFGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANFGameState, DeliveredStrongboxes);
	DOREPLIFETIME(ANFGameState, Quota);
	DOREPLIFETIME(ANFGameState, MissionResult);
}

void ANFGameState::AuthAddDeliveredStrongbox()
{
	if (HasAuthority())
	{
		++DeliveredStrongboxes;
		UE_LOG(LogNewFPS, Log, TEXT("Strongbox delivered: %d / %d"), DeliveredStrongboxes, Quota);
		ForceNetUpdate();
	}
}

void ANFGameState::AuthSetQuota(int32 NewQuota)
{
	if (HasAuthority())
	{
		Quota = FMath::Max(NewQuota, 1);
		ForceNetUpdate();
	}
}

void ANFGameState::AuthSetMissionResult(ENFMissionResult NewResult)
{
	if (HasAuthority() && MissionResult != NewResult)
	{
		MissionResult = NewResult;
		ForceNetUpdate();
	}
}
