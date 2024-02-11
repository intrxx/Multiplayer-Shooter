// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BCaptureTheFlagGameMode.h"

void ABCaptureTheFlagGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABPlayerController* TargetBPC,
	ABPlayerController* AttackerBPC)
{
	ABlasterGameMode::PlayerEliminated(ElimmedCharacter, TargetBPC, AttackerBPC);
	
}
