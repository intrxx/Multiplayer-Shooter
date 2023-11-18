// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BPlayerController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "HUD/BCharacterOverlay.h"
#include "HUD/BlasterHUD.h"
#include "Character/BlasterCharacter.h"
#include "Components/Image.h"
#include "HUD/BScoreBoard.h"
#include "Components/Overlay.h"
#include "HUD/BInventoryWidget.h"
#include "Net/UnrealNetwork.h"


void ABPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

void ABPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetHUDGameTime();
	CheckTimeSync(DeltaSeconds);
}

void ABPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	// Sync with server clock as soon as possible
	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
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

void ABPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimerServerReceivedClientRequest)
{
	// Time from sending the request to server and receiving the answer from server
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimerServerReceivedClientRequest + (0.5f * RoundTripTime);

	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ABPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABPlayerController, LocalPlayerStats);
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

void ABPlayerController::SetHUDCarriedAmmo(int32 CarriedAmmo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount;
		
	if(bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), CarriedAmmo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABPlayerController::SetHUDInventoryCarriedAmmo(EBWeaponType WeaponType, int32 CarriedAmmo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	if(BlasterHUD && BlasterHUD->Inventory)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), CarriedAmmo);
		switch (WeaponType)
		{
		case EBWeaponType::EWT_AssaultRifle:
			if(BlasterHUD->Inventory->RifleWeaponAmmoAmount)
			{
				BlasterHUD->Inventory->RifleWeaponAmmoAmount->SetText(FText::FromString(AmmoText));
			}
			break;
		default:
			break;
		}
	}
	
}

void ABPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount;
		
	if(bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABPlayerController::SetHUDWeaponAmmoImage(EBFiringMode FireMode)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->AmmoImage &&
		BlasterHUD->CharacterOverlay->SingleAmmoImage &&
		BlasterHUD->CharacterOverlay->BurstAmmoImage &&
		BlasterHUD->CharacterOverlay->AutomaticAmmoImage;
		
	if(bHUDValid)
	{
		switch(FireMode)
		{
		case EBFiringMode::EFM_SingleBullet:
			BlasterHUD->CharacterOverlay->AmmoImage->SetBrushFromTexture(BlasterHUD->CharacterOverlay->SingleAmmoImage);
			break;
		case EBFiringMode::EFM_Burst:
			BlasterHUD->CharacterOverlay->AmmoImage->SetBrushFromTexture(BlasterHUD->CharacterOverlay->BurstAmmoImage);
			break;
		case EBFiringMode::EFM_FullAuto:
			BlasterHUD->CharacterOverlay->AmmoImage->SetBrushFromTexture(BlasterHUD->CharacterOverlay->AutomaticAmmoImage);
			break;
		default:
			break;
		}
	}
}

void ABPlayerController::SetHUDWeaponTypeText(EBWeaponType WeaponType)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponType;
		
	if(bHUDValid)
	{
		switch(WeaponType)
		{
		case EBWeaponType::EWT_AssaultRifle:
			BlasterHUD->CharacterOverlay->WeaponType->SetText(FText::FromString(TEXT("Rifle")));
			break;
		default:
			BlasterHUD->CharacterOverlay->WeaponType->SetText(FText::FromString(TEXT("-")));
			break;
		}
	}
}

void ABPlayerController::SetHUDGameTimer(float Time)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->GameTimerText;
	if(bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(Time / 60.f);
		int32 Seconds = Time - Minutes * 60;
		
		FString CountdownText = FString::Printf(TEXT("%02d : %02d"), Minutes, Seconds); 
		BlasterHUD->CharacterOverlay->GameTimerText->SetText(FText::FromString(CountdownText));
	}
}

float ABPlayerController::GetServerTimeSeconds()
{
	if(HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABPlayerController::ClientSetHUDDeathScreen_Implementation(const FString& KillerName)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DeathInfoOverlay &&
		BlasterHUD->CharacterOverlay->DamageDone &&
		BlasterHUD->CharacterOverlay->DamageTaken &&
		BlasterHUD->CharacterOverlay->KilledByName;
	if(bHUDValid)
	{
		BlasterHUD->CharacterOverlay->KilledByName->SetText(FText::FromString(KillerName));
		SetDeathScreenVisibility(true);
	}
}

void ABPlayerController::SetDeathScreenVisibility(bool bSetVisibility)
{
	if(BlasterHUD && BlasterHUD->CharacterOverlay)
	{
		if(bSetVisibility)
		{
			BlasterHUD->CharacterOverlay->DeathInfoOverlay->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			BlasterHUD->CharacterOverlay->DeathInfoOverlay->SetVisibility(ESlateVisibility::Collapsed);
		}
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


void ABPlayerController::SetHUDGameTime()
{
	uint32 SecondLeft = FMath::CeilToInt(MatchTimer - GetServerTimeSeconds());

	if(CountDown != SecondLeft)
	{
		SetHUDGameTimer(MatchTimer - GetServerTimeSeconds());
	}
	CountDown = SecondLeft;
}

void ABPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if(IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABPlayerController::ClientSetHUDPlayerStats_Implementation(const TArray<FPlayerStats>& PlayerStats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Scoreboard;
	
	if(bHUDValid)
	{
		for(FPlayerStats PStats : PlayerStats)
		{
			BlasterHUD->Scoreboard->UpdatePlayerList(PlayerStats);
		}
	}
}

void ABPlayerController::OnRep_PlayerStats()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Scoreboard;

	if(bHUDValid)
	{
		for(FPlayerStats PStats : LocalPlayerStats)
		{
			BlasterHUD->Scoreboard->UpdatePlayerList(LocalPlayerStats);
		}
	}
}




