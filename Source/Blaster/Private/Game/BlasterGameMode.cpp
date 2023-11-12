// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BlasterGameMode.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Game/BlasterGameState.h"
#include "Player/BPlayerState.h"

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABPlayerController* TargetBPC,
                                        ABPlayerController* AttackerBPC)
{
	ABPlayerState* AttackerPS = AttackerBPC ? Cast<ABPlayerState>(AttackerBPC->PlayerState) : nullptr;
	ABPlayerState* TargetPS = TargetBPC ? Cast<ABPlayerState>(TargetBPC->PlayerState) : nullptr;

	if(AttackerPS && AttackerPS != TargetPS)
	{
		AttackerPS->AddToScore(KillScoreAward);
		AttackerPS->AddToKills(1);
		UpdatePlayerList();
	}
	
	if(TargetPS)
	{
		TargetPS->AddToDeaths(1);
		UpdatePlayerList();
	}
	
	if(ElimmedCharacter)
	{
		ElimmedCharacter->HandleDeath();
	}
}

void ABlasterGameMode::RequestRespawn(ABlasterCharacter* CharacterToRespawn, AController* TargetBPC)
{
	if(CharacterToRespawn)
	{
		CharacterToRespawn->Reset();
		CharacterToRespawn->Destroy();
	}
	
	if(TargetBPC)
	{
		AActor* SelectedPlayerStart;
		CalculateFurthestSpawnLocation(/*OUT*/ SelectedPlayerStart);
		
		RestartPlayerAtPlayerStart(TargetBPC, SelectedPlayerStart);
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

void ABlasterGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	LoginPlayerControllers.Add(NewPlayer);
	UpdatePlayerList();
}

void ABlasterGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	LoginPlayerControllers.Remove(Exiting);
	UpdatePlayerList();
}

void ABlasterGameMode::UpdatePlayerList()
{
	PlayerStats.Empty();
	
	for(AController* C : LoginPlayerControllers)
	{
		ABPlayerState* PS = C->GetPlayerState<ABPlayerState>();
		if(PS)
		{
			FPlayerStats NewPlayerStats;
			NewPlayerStats.PlayerName = PS->GetPlayerName();
			NewPlayerStats.Score = PS->GetScore();
			NewPlayerStats.Deaths = PS->GetDeaths();
			NewPlayerStats.Kills = PS->GetKills();
			PlayerStats.Add(NewPlayerStats);
		}
	}

	for(AController* C  : LoginPlayerControllers)
	{
		ABPlayerController* PC = Cast<ABPlayerController>(C);
		{
			if (PC)
			{
				//PC->SetPlayerStats(PlayerStats);
				PC->ClientSetHUDPlayerStats(PlayerStats);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("PC is not valid for some reason"));
			}
		}
	}
}
