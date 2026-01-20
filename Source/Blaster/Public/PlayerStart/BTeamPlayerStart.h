// Copyright 2026 out of sCope team - intrxx

#pragma once

#include "CoreMinimal.h"
#include "BlasterTypes/BTeams.h"
#include "GameFramework/PlayerStart.h"
#include "BTeamPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABTeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Blaster|TeamPlayerStart")
	EBTeam Team;
};
