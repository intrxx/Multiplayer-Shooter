// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

class ABPlayerState;
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

	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

private:
	float TopScore = 0.f;

	UFUNCTION()
	void OnRep_RedTeamScore();

	UFUNCTION()
	void OnRep_BlueTeamScore();
};

