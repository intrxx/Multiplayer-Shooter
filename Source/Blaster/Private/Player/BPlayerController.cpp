// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BPlayerController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "HUD/BCharacterOverlay.h"
#include "HUD/BlasterHUD.h"
#include "Character/BlasterCharacter.h"
#include "HUD/BScoreBoard.h"
#include "Net/UnrealNetwork.h"


void ABPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

void ABPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if(BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHeath(), BlasterCharacter->GetMaxHeath());
	}
}

void ABPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABPlayerController, LocalPlayerStats)
}

void ABPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthProgressBar &&
		BlasterHUD->CharacterOverlay->HealthText;
	if(bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthProgressBar->SetPercent(HealthPercent);

		FString HealthText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Health));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

/*
void ABPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Scoreboard; //&&
		BlasterHUD->Scoreboard->ScoreAmount;
	if(bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->Scoreboard->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}
*/

void ABPlayerController::ClientSetHUDPlayerStats_Implementation(const TArray<FPlayerStats>& PlayerStats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Scoreboard;
	
	if(bHUDValid)
	{
		if(PlayerStats.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("PlayerStats is empty on %s"), *GetNetOwningPlayer()->GetName());
			return;
		}
		
		for(FPlayerStats PStats : PlayerStats)
		{
			BlasterHUD->Scoreboard->UpdatePlayerList(PlayerStats);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No HUD on: %s"), *GetNetOwningPlayer()->GetName());
	}
}

void ABPlayerController::OnRep_PlayerStats()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Scoreboard;

	if(bHUDValid)
	{
		if(LocalPlayerStats.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("PlayerStats is empty on %s"), *GetNetOwningPlayer()->GetName());
			return;
		}
		
		for(FPlayerStats PStats : LocalPlayerStats)
		{
			BlasterHUD->Scoreboard->UpdatePlayerList(LocalPlayerStats);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No HUD on: %s"), *GetNetOwningPlayer()->GetName());
	}
}



