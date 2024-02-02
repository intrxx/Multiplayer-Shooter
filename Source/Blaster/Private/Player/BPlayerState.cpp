// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BPlayerState.h"

#include "Character/BlasterCharacter.h"
#include "Game/BlasterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


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
		const float PlayerCount = BlasterGameState->PlayerArray.Num();
		const float TeamLimit = FMath::CeilToInt(PlayerCount / 2);
		
		if(TeamToSet == EBTeam::EBT_RedTeam)
		{
			if(BlasterGameState->RedTeam.Num() == TeamLimit)
			{
				return;
			}
			
			RemoveFromOtherTeams(EBTeam::EBT_RedTeam);
			
			BlasterGameState->RedTeam.AddUnique(this);
			SetTeam(EBTeam::EBT_RedTeam);
		}

		if(TeamToSet == EBTeam::EBT_BlueTeam)
		{
			if(BlasterGameState->BlueTeam.Num() == TeamLimit)
			{
				return;
			}
			
			RemoveFromOtherTeams(EBTeam::EBT_BlueTeam);
			
			BlasterGameState->BlueTeam.AddUnique(this);
			SetTeam(EBTeam::EBT_BlueTeam);
		}

		if(TeamToSet == EBTeam::EBT_ChooseRandomTeam)
		{
			if(BlasterGameState->RandomTeam.Num() == PlayerCount)
			{
				return;
			}
			
			RemoveFromOtherTeams(EBTeam::EBT_ChooseRandomTeam);
			
			BlasterGameState->RandomTeam.AddUnique(this);
			SetTeam(EBTeam::EBT_ChooseRandomTeam);
		}

		BlasterGameState->UpdateHUDTeamSelect();

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

void ABPlayerState::RemoveFromOtherTeams(EBTeam ChosenTeam)
{
	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if(BlasterGameState == nullptr)
	{
		return;
	}
	
	if(ChosenTeam == EBTeam::EBT_BlueTeam)
	{
		if(BlasterGameState->RedTeam.Contains(this))
		{
			BlasterGameState->RedTeam.Remove(this);
		}

		if(BlasterGameState->RandomTeam.Contains(this))
		{
			BlasterGameState->RandomTeam.Remove(this);
		}
	}
	
	if(ChosenTeam == EBTeam::EBT_RedTeam)
	{
		if(BlasterGameState->BlueTeam.Contains(this))
		{
			BlasterGameState->BlueTeam.Remove(this);
		}

		if(BlasterGameState->RandomTeam.Contains(this))
		{
			BlasterGameState->RandomTeam.Remove(this);
		}
	}

	if(ChosenTeam == EBTeam::EBT_ChooseRandomTeam)
	{
		if(BlasterGameState->BlueTeam.Contains(this))
		{
			BlasterGameState->BlueTeam.Remove(this);
		}
		
		if(BlasterGameState->RedTeam.Contains(this))
		{
			BlasterGameState->RedTeam.Remove(this);
		}
	}
}

void ABPlayerState::OnRep_Team()
{
}





