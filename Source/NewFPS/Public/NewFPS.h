#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

NEWFPS_API DECLARE_LOG_CATEGORY_EXTERN(LogNewFPS, Log, All);

/** Weapon trace channel. Must match the "Weapon" channel declared in Config/DefaultEngine.ini. */
#define NF_ECC_Weapon ECC_GameTraceChannel1
