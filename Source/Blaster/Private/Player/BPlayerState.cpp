// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BPlayerState.h"

#include "Game/BlasterGameState.h"
#include "Game/BTeamsGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/BPlayerController.h"


void ABPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABPlayerState, Team);
}

void ABPlayerState::AddToScore(float ScoreToAdd)
{
	SetScore(GetScore() + ScoreToAdd);
}

void ABPlayerState::AddToDeaths(int32 DeathToAdd)
{
	Deaths += DeathToAdd;
}

void ABPlayerState::AddToKills(int32 KillsToAdd)
{
	Kills += KillsToAdd;
}

void ABPlayerState::AddToAssists(int32 AssistsToAdd)
{
	Assists += AssistsToAdd;
}

void ABPlayerState::ServerSetTeam_Implementation(EBTeam TeamToSet)
{
	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if(BlasterGameState)
	{
		if(TeamToSet == EBTeam::EBT_RedTeam)
		{
			BlasterGameState->RedTeam.AddUnique(this);
			SetTeam(EBTeam::EBT_RedTeam);
		}

		if(TeamToSet == EBTeam::EBT_BlueTeam)
		{
			BlasterGameState->BlueTeam.AddUnique(this);
			SetTeam(EBTeam::EBT_BlueTeam);
		}

		BlasterGameState->UpdatePlayerHUDCountNumber(TeamToSet);

		/**
		 *TODO fix the time issues
		 *
		if(BlasterGameState->AllPlayersChosenTeam())
		{
			ABTeamsGameMode* TeamsGameMode = Cast<ABTeamsGameMode>(UGameplayStatics::GetGameMode(this));
			if(TeamsGameMode)
			{
				if(TeamsGameMode->GetCountdownTime() > 10.f)
				{
					TeamsGameMode->WarmupTime = 10.f;
				}
			}
		}
		*/
	}
}





