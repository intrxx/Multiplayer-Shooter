// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/BTeamsGameMode.h"
#include "BCaptureTheFlagGameMode.generated.h"

class ABFlagZone;
class ABFlag;
/**
 * 
 */
UCLASS()
class BLASTER_API ABCaptureTheFlagGameMode : public ABTeamsGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABPlayerController* TargetBPC, ABPlayerController* AttackerBPC) override;

	void FlagCaptured(const ABFlag* Flag, const ABFlagZone* FlagZone);
};