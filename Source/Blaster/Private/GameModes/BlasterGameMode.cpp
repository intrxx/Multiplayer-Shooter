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
		AActor* SelectedPlayerStart;
		CalculateFurthestSpawnLocation(/*OUT*/ SelectedPlayerStart);
		
		RestartPlayerAtPlayerStart(ElimmedBPC, SelectedPlayerStart);
	}
}

void ABlasterGameMode::CalculateFurthestSpawnLocation(AActor*& OutSpawnPoint)
{
	TArray<AActor*> PlayerStarts;
	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), /*OUT*/ PlayerStarts);
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABlasterCharacter::StaticClass(), /*OUT*/ Players);
		
	float FurthestDistance = 0.f;
	OutSpawnPoint = PlayerStarts[0];
	
	for(int32 i = 0; i < PlayerStarts.Num(); i++)
	{
		FVector PlayerStartLocation = PlayerStarts[i]->GetActorLocation();
		float Distance = 0.f;
			
		for(int32 j = 0; j < Players.Num(); j++)
		{
			FVector PlayerLocation = Players[j]->GetActorLocation();
			Distance += (PlayerStartLocation - PlayerLocation).Size();
		}
		if(Distance > FurthestDistance)
		{
			FurthestDistance = Distance;
			OutSpawnPoint = PlayerStarts[i];
		}
	}
}
