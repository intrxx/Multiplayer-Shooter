// Copyright 2026 out of sCope team - intrxx


#include "Game/BTeamsGameMode.h"
#include "Game/BlasterGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/BPlayerState.h"
#include "Player/BPlayerController.h"


void ABTeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABPlayerController* BPC = Cast<ABPlayerController>(NewPlayer);
	if(BPC)
	{
		BPC->ClientCreateTeamSelectWidget();
	}
	
	/*
	 * Original code
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

void ABTeamsGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABPlayerController* TargetBPC,
	ABPlayerController* AttackerBPC)
{
	Super::PlayerEliminated(ElimmedCharacter, TargetBPC, AttackerBPC);

	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	ABPlayerState* AttackerPlayerState = AttackerBPC ? Cast<ABPlayerState>(AttackerBPC->PlayerState) : nullptr;
	ABPlayerState* TargetPlayerState = TargetBPC ? Cast<ABPlayerState>(TargetBPC->PlayerState) : nullptr;
	if(BGameState && AttackerPlayerState && TargetPlayerState)
	{
		if(AttackerPlayerState->GetTeam() != TargetPlayerState->GetTeam())
		{
			if(AttackerPlayerState->GetTeam() == EBTeam::EBT_BlueTeam)
			{
				BGameState->BlueTeamScores(1.f);
			}

			if(AttackerPlayerState->GetTeam() == EBTeam::EBT_RedTeam)
			{
				BGameState->RedTeamScores(1.f);
			}
		}
		else
		{
			if(AttackerPlayerState->GetTeam() == EBTeam::EBT_BlueTeam)
			{
				BGameState->BlueTeamScores(-1.f);
			}

			if(AttackerPlayerState->GetTeam() == EBTeam::EBT_RedTeam)
			{
				BGameState->RedTeamScores(-1.f);
			}
		}	
	}
}

float ABTeamsGameMode::CalculateDamage(AController* Source, AController* Target, float BaseDamage)
{
	ABPlayerState* SourcePS = Source->GetPlayerState<ABPlayerState>();
	ABPlayerState* TargetPS = Target->GetPlayerState<ABPlayerState>();

	if(SourcePS == nullptr || TargetPS == nullptr)
	{
		return BaseDamage;
	}
	
	if(SourcePS == TargetPS)
	{
		return BaseDamage;
	}

	if(SourcePS->GetTeam() == TargetPS->GetTeam())
	{
		return FMath::RoundToInt(BaseDamage / 4.f);
	}

	return BaseDamage;
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
	
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if(BGameState)
	{
		for(auto PS : BGameState->PlayerArray)
		{
			ABPlayerState* BPS = Cast<ABPlayerState>(PS);
			if(BPS && (BPS->GetTeam() == EBTeam::EBT_NoTeam || BPS->GetTeam() == EBTeam::EBT_ChooseRandomTeam))
			{
				if(BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					BGameState->RedTeam.AddUnique(BPS);
					BPS->SetTeam(EBTeam::EBT_RedTeam);
					BPS->bHasTeam = true;
				}
				else
				{
					BGameState->BlueTeam.AddUnique(BPS);
					BPS->SetTeam(EBTeam::EBT_BlueTeam);
					BPS->bHasTeam = true;
				}
			}
		}
	}
}
