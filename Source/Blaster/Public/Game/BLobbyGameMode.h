// Copyright 2026 out of sCope team - intrxx

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BLobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABLobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
