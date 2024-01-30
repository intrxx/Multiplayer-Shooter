// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BlasterGameState.h"

#include "Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Player/BPlayerState.h"


void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
}

void ABlasterGameState::UpdateTopScore(ABPlayerState* ScoringPlayer)
{
	if(TopScoringPlayers.IsEmpty())
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if(ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if(ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}

	DisplayTeams();
}

void ABlasterGameState::OnRep_RedTeamScore()
{
	
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
	
}

void ABlasterGameState::DisplayTeams()
{
	for(const auto& PS : RedTeam)
	{
		if(PS)
		{
			UE_LOG(LogTemp, Warning, TEXT("Red Team Member: %s"), *PS->GetName());
		}
	}

	for(const auto& PS : BlueTeam)
	{
		if(PS)
		{
			UE_LOG(LogTemp, Warning, TEXT("Blue Team Member: %s"), *PS->GetName());
		}
	}
}

