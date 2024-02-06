// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterTypes/BTeams.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

class ABPlayerState;
class ABlasterCharacter;
/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()

public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(ABPlayerState* ScoringPlayer);
	void UpdateHUDTeamSelect();
	bool AllPlayersChosenTeam();

	void RedTeamScores(float ScoreMagnitude);
	void BlueTeamScores(float ScoreMagnitude);

public:
	UPROPERTY(Replicated)
	TArray<TObjectPtr<ABPlayerState>> TopScoringPlayers;

	/**
	 * Teams
	 */
	
	UPROPERTY()
	TArray<TObjectPtr<ABPlayerState>> RedTeam;
	
	UPROPERTY()
	TArray<TObjectPtr<ABPlayerState>> BlueTeam;
	
	UPROPERTY()
	TArray<TObjectPtr<ABPlayerState>> RandomTeam;
	
	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

private:
	UFUNCTION()
	void OnRep_RedTeamScore();

	UFUNCTION()
	void OnRep_BlueTeamScore();
	
	void DisplayTeams();
	
private:
	float TopScore = 0.f;
};

