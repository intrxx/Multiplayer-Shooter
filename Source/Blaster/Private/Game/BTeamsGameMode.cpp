// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BTeamsGameMode.h"
#include "Game/BlasterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BPlayerState.h"
#include "Player/BPlayerController.h"

void ABTeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	/*
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if(BGameState)
	{
		ABPlayerState* BPS = NewPlayer->GetPlayerState<ABPlayerState>();
		if(BPS && BPS->GetTeam() == EBTeam::EBT_NoTeam)
		{
			if(BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(BPS);
				BPS->SetTeam(EBTeam::EBT_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.AddUnique(BPS);
				BPS->SetTeam(EBTeam::EBT_BlueTeam);
			}
		}
	}
	*/
}

void ABTeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABPlayerState* BPS = Exiting->GetPlayerState<ABPlayerState>();
	if(BGameState && BPS)
	{
		if(BGameState->RedTeam.Contains(BPS))
		{
			BGameState->RedTeam.Remove(BPS);
		}
		else if (BGameState->BlueTeam.Contains(BPS))
		{
			BGameState->BlueTeam.Remove(BPS);
		}
	}
}

void ABTeamsGameMode::OnMatchStateSet()
{
	AGameMode::OnMatchStateSet();

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABPlayerController* BPC = Cast<ABPlayerController>(*It);
		if(BPC)
		{
			BPC->OnMatchStateSet(MatchState);
			BPC->bIsTeamsMatch = true;
		}
	}
}

void ABTeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	/*
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if(BGameState)
	{
		for(auto PS :BGameState->PlayerArray)
		{
			ABPlayerState* BPS = Cast<ABPlayerState>(PS);
			if(BPS && BPS->GetTeam() == EBTeam::EBT_NoTeam)
			{
				if(BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(BPS);
					BPS->SetTeam(EBTeam::EBT_RedTeam);
				}
				else
				{
					BGameState->BlueTeam.AddUnique(BPS);
					BPS->SetTeam(EBTeam::EBT_BlueTeam);
				}
			}
		}
	}
	*/
}
