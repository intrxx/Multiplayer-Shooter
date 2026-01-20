// Copyright 2026 out of sCope team - intrxx

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
