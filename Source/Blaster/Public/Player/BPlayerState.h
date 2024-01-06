// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BPlayerState.generated.h"

class ABPlayerController;
class ABlasterCharacter;

/**
 * 
 */
UCLASS()
class BLASTER_API ABPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	void AddToScore(float ScoreToAdd);
	void AddToDeaths(int32 DeathToAdd);
	void AddToKills(int32 KillsToAdd);
	void AddToAssists(int32 AssistsToAdd);
	void UpdatePingStat(float PingToUpdate);

	int32 GetKills() const {return Kills;}
	int32 GetDeaths() const {return Deaths;}
	
private:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> BlasterCharacter;

	UPROPERTY()
	TObjectPtr<ABPlayerController> BlasterPC;

	int32 Deaths;
	int32 Kills;
	int32 Assists;
};
