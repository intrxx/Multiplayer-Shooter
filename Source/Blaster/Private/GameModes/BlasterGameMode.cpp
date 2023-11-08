// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/BlasterGameMode.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BPlayerController.h"
#include "GameFramework/PlayerStart.h"

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABPlayerController* ElimmedBPC,
	ABPlayerController* AttackerBPC)
{
	if(ElimmedCharacter)
	{
		ElimmedCharacter->HandleDeath();
	}
}

void ABlasterGameMode::RequestRespawn(ABlasterCharacter* CharacterToRespawn, AController* ElimmedBPC)
{
	if(CharacterToRespawn)
	{
		CharacterToRespawn->Reset();
		CharacterToRespawn->Destroy();
	}
	
	if(ElimmedBPC)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), /*OUT*/ PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		
		RestartPlayerAtPlayerStart(ElimmedBPC, PlayerStarts[Selection]);
	}
}
