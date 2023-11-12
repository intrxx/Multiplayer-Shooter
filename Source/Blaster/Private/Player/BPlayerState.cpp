// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BPlayerState.h"
#include "GameModes/BlasterGameMode.h"
#include "Kismet/GameplayStatics.h"


void ABPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if(BlasterGameMode)
	{
		BlasterGameMode->UpdatePlayerList();
	}
}

void ABPlayerState::AddToScore(float ScoreToAdd)
{
	SetScore(GetScore() + ScoreToAdd);
	
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if(BlasterGameMode)
	{
		BlasterGameMode->UpdatePlayerList();
	}
}
