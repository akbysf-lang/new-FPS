#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NFPlayerController.generated.h"

/** Player controller for the prototype. Input bindings live on the character. */
UCLASS()
class NEWFPS_API ANFPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
