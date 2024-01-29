// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/BlasterGameMode.h"
#include "BTeamsGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABTeamsGameMode : public ABlasterGameMode
{
	GENERATED_BODY()
public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	
protected:
	virtual void HandleMatchHasStarted() override;
};
