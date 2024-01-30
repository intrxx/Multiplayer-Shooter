// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Player/BPlayerController.h"
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

void ABlasterGameState::UpdatePlayerHUDCountNumber(const EBTeam TeamToSet)
{
	for(const auto& PS : PlayerArray)
	{
		ABPlayerController* BPC = Cast<ABPlayerController>(PS->GetPlayerController());
		if(BPC)
		{
			switch(TeamToSet)
			{
			case EBTeam::EBT_RedTeam:
				BPC->ClientSetPlayerHUDCountInTeam(TeamToSet, RedTeam.Num());
				break;
			case EBTeam::EBT_BlueTeam:
				BPC->ClientSetPlayerHUDCountInTeam(TeamToSet, BlueTeam.Num());
				break;
			default:
				break;
			}
		
		}
	}
}

bool ABlasterGameState::AllPlayersChosenTeam()
{
	for(const auto& PS : PlayerArray)
	{
		ABPlayerState* BPS = Cast<ABPlayerState>(PS);
		if(BPS && BPS->GetTeam() == EBTeam::EBT_NoTeam)
		{
			return false;
		}
	}
	return true;
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

