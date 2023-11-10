// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BPlayerState.h"
#include "Character/BlasterCharacter.h"
#include "Player/BPlayerController.h"

void ABPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : BlasterCharacter;
	if(BlasterCharacter)
	{
		BlasterPC = BlasterPC == nullptr ? Cast<ABPlayerController>(BlasterCharacter->Controller) : BlasterPC;
		if(BlasterPC)
		{
			BlasterPC->SetHUDScore(GetScore());
		}
	}
}

void ABPlayerState::AddToScore(float ScoreToAdd)
{
	SetScore(GetScore() + ScoreToAdd);
	
	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : BlasterCharacter;
	if(BlasterCharacter)
	{
		BlasterPC = BlasterPC == nullptr ? Cast<ABPlayerController>(BlasterCharacter->Controller) : BlasterPC;
		if(BlasterPC)
		{
			BlasterPC->SetHUDScore(GetScore());
		}
	}
}
