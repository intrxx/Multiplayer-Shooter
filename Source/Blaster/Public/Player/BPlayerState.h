// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterTypes/BTeams.h"
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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	int32 GetKills() const {return Kills;}
	int32 GetDeaths() const {return Deaths;}
	EBTeam GetTeam() const {return Team;}

	void SetTeam(EBTeam TeamToSet) {Team = TeamToSet;}

	UFUNCTION(Server, Reliable)
	void ServerSetTeam(EBTeam TeamToSet);

private:
	void RemoveFromOtherTeams(EBTeam ChosenTeam);
	
private:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> BlasterCharacter;

	UPROPERTY()
	TObjectPtr<ABPlayerController> BlasterPC;

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	EBTeam Team = EBTeam::EBT_NoTeam;

	UFUNCTION()
	void OnRep_Team();
	
	int32 Deaths;
	int32 Kills;
	int32 Assists;
};
