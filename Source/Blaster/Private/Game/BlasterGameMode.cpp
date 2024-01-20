// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BlasterGameMode.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Game/BlasterGameState.h"
#include "Player/BPlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName(TEXT("Cooldown"));
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartedTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartedTime;
		if(CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if(MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartedTime;
		if(CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if(MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartedTime;
		if(CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABPlayerController* BPC = Cast<ABPlayerController>(*It);
		if(BPC)
		{
			BPC->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABPlayerController* TargetBPC,
                                        ABPlayerController* AttackerBPC)
{
	ABPlayerState* AttackerPS = AttackerBPC ? Cast<ABPlayerState>(AttackerBPC->PlayerState) : nullptr;
	ABPlayerState* TargetPS = TargetBPC ? Cast<ABPlayerState>(TargetBPC->PlayerState) : nullptr;

	ABPlayerController* TargetController = Cast<ABPlayerController>(ElimmedCharacter->Controller);

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	if(AttackerPS && AttackerPS != TargetPS && BlasterGameState)
	{
		AttackerPS->AddToScore(KillScoreAward);
		AttackerPS->AddToKills(1);
		UpdatePlayerList();

		BlasterGameState->UpdateTopScore(AttackerPS);
	}
	
	if(TargetPS)
	{
		TargetPS->AddToDeaths(1);
		UpdatePlayerList();
	}
	
	if(ElimmedCharacter)
	{
		if(TargetController)
		{
			FString KillerName = TargetPS->GetPlayerName();
			TargetController->ClientSetHUDDeathScreen(KillerName);
		}
		ElimmedCharacter->HandleDeath(false);
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

		if(ABPlayerController* BPC = Cast<ABPlayerController>(TargetBPC))
		{
			BPC->SetDeathScreenVisibility(false);
		}
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

void ABlasterGameMode::PlayerLeftGame(ABPlayerState* LeavingPlayer)
{
	if(LeavingPlayer == nullptr)
	{
		return;
	}
	
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if(BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(LeavingPlayer))
	{
		BlasterGameState->TopScoringPlayers.Remove(LeavingPlayer);
	}

	ABlasterCharacter* BlasterPlayerLeaving = Cast<ABlasterCharacter>(LeavingPlayer->GetPawn());
	if(BlasterPlayerLeaving)
	{
		BlasterPlayerLeaving->HandleDeath(true);
	}
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
			NewPlayerStats.Ping = FMath::FloorToInt(PS->GetPingInMilliseconds());
			PlayerStats.Add(NewPlayerStats);
		}
	}

	for(AController* C  : LoginPlayerControllers)
	{
		ABPlayerController* PC = Cast<ABPlayerController>(C);
		{
			if (PC)
			{
				PC->ClientSetHUDPlayerStats(PlayerStats);
			}
		}
	}
}
