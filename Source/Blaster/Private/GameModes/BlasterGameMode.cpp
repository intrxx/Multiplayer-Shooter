// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/BlasterGameMode.h"
#include "Character/BlasterCharacter.h"
#include "Player/BPlayerController.h"

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABPlayerController* ElimmedBPC,
	ABPlayerController* AttackerBPC)
{
	UE_LOG(LogTemp, Warning, TEXT("Player Eliminated"));
}
