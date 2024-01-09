// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BPlayerController.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "HUD/BCharacterOverlay.h"
#include "HUD/BlasterHUD.h"
#include "Character/BlasterCharacter.h"
#include "Components/Image.h"
#include "HUD/BScoreBoard.h"
#include "EnhancedInputSubsystems.h"
#include "Components/Overlay.h"
#include "HUD/BInventoryWidget.h"
#include "Net/UnrealNetwork.h"
#include "Game/BlasterGameMode.h"
#include "HUD/BAnnouncement.h"
#include "Kismet/GameplayStatics.h"
#include "BlasterComponents/BCombatComponent.h"
#include "Game/BlasterGameState.h"
#include "Player/BPlayerState.h"
#include "Weapon/BGrenade.h"

void ABPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	ServerCheckMatchState();
}

void ABPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	SetGameTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();

	CheckPing(DeltaSeconds);
}

void ABPlayerController::CheckPing(float DeltaSeconds)
{
	HighPingRunningTime += DeltaSeconds;
	if(HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if(PlayerState)
		{
			if(PlayerState->GetPingInMilliseconds() > HighPingThreshold) 
			{
				StartHighPingWarning();
				
				PingAnimationRunningTime = 0.f;
			}
		}

		HighPingRunningTime = 0.f;
	}

	if(BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HighPingAnimation && BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation))
	{
		PingAnimationRunningTime += DeltaSeconds;
		if(PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
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
		SetHUDShield(BlasterCharacter->GetShield(), BlasterCharacter->GetMaxShield());
	}
}

void ABPlayerController::PollInit()
{
	if(CharacterOverlay == nullptr)
	{
		if(BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if(CharacterOverlay)
			{
				if(bInitHealth)
				{
					SetHUDHealth(HUDHealth, HUDMaxHealth);
				}

				if(bInitShield)
				{
					SetHUDShield(HUDShield, HUDMaxShield);
				}

				if(bInitCarriedAmmo)
				{
					SetHUDCarriedAmmo(CarriedAmmo);
				}

				if(bInitInventoryAmmo)
				{
					SetHUDInventoryCarriedAmmo(InventoryWeaponType, InventoryAmmo);
				}

				if(bInitWeaponAmmo)
				{
					SetHUDWeaponAmmo(WeaponAmmo);
				}

				if(bInitWeaponImage)
				{
					SetHUDWeaponAmmoImage(FiringMode);
				}
				
				//if(bInitGrenades)
				//{
					if(ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn()))
					{
						if(BlasterCharacter->GetCombatComp())
						{
							SetHUDGrenadesNumber(BlasterCharacter->GetCombatComp()->GetLethalGrenades(), EBGrenadeCategory::EGC_Lethal);
							SetHUDGrenadesNumber(BlasterCharacter->GetCombatComp()->GetTacticalGrenades(), EBGrenadeCategory::EGC_Tactical);

							if(BlasterCharacter->GetCombatComp()->GetEquippedLethalGrenade())
							{
								SetHUDGrenadesImage(BlasterCharacter->GetCombatComp()->GetEquippedLethalGrenade()->GrenadeHUDImage, EBGrenadeCategory::EGC_Lethal);
							}
							if(BlasterCharacter->GetCombatComp()->GetEquippedTacticalGrenade())
							{
								SetHUDGrenadesImage(BlasterCharacter->GetCombatComp()->GetEquippedTacticalGrenade()->GrenadeHUDImage, EBGrenadeCategory::EGC_Tactical);
							}
						}
					}
				//}
			}
		}
	}

	if(Scoreboard == nullptr)
	{
		if(BlasterHUD && BlasterHUD->Scoreboard)
		{
			Scoreboard = BlasterHUD->Scoreboard;
			if(Scoreboard)
			{
				for(FPlayerStats Stats : LocalPlayerStats)
				{
					BlasterHUD->Scoreboard->UpdatePlayerList(LocalPlayerStats);
				}
			}
		}
	}
	
	if(Announcement == nullptr && HasAuthority())
	{
		if(BlasterHUD)
		{
			BlasterHUD->AddAnnouncement();
			if(BlasterHUD->Announcement)
			{
				Announcement = BlasterHUD->Announcement;
			}
		}
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
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimerServerReceivedClientRequest + SingleTripTime;

	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ABPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ABPlayerController, MatchState);
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
	else
	{
		bInitHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ShieldProgressBar &&
		BlasterHUD->CharacterOverlay->ShieldText;
	if(bHUDValid)
	{
		const float ShieldPercent = Shield / MaxShield;
		BlasterHUD->CharacterOverlay->ShieldProgressBar->SetPercent(ShieldPercent);

		FString ShieldText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Shield));
		BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		bInitShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

void ABPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount;
		
	if(bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitCarriedAmmo = true;
		CarriedAmmo = Ammo;
	}	
}

void ABPlayerController::SetHUDInventoryCarriedAmmo(EBWeaponType WeaponType, int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	if(BlasterHUD && BlasterHUD->Inventory)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
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
	else
	{
		bInitInventoryAmmo = true;
		InventoryWeaponType = WeaponType;
		InventoryAmmo = Ammo;
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
	else
	{
		bInitWeaponAmmo = true;
		WeaponAmmo = Ammo;
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
	else
	{
		bInitWeaponImage = true;
		FiringMode = FireMode;
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
		case EBWeaponType::EWT_RocketLauncher:
			BlasterHUD->CharacterOverlay->WeaponType->SetText(FText::FromString(TEXT("Launcher")));
			break;
		default:
			BlasterHUD->CharacterOverlay->WeaponType->SetText(FText::FromString(TEXT("-")));
			break;
		}
	}
}

void ABPlayerController::SetHUDGameTimer(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->GameTimerText;
	if(bHUDValid)
	{
		if(CountdownTime <  0.f)
		{
			BlasterHUD->CharacterOverlay->GameTimerText->SetText(FText::FromString(""));
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		
		FString CountdownText = FString::Printf(TEXT("%02d : %02d"), Minutes, Seconds);
		FSlateColor TextColor;
		if(Minutes == 0 && Seconds <= 15)
		{
			TextColor = FLinearColor(1.f, 0.f, 0.f);
		}
		else
		{
			TextColor = FLinearColor(1.f, 1.f, 1.f);
		}
		BlasterHUD->CharacterOverlay->GameTimerText->SetColorAndOpacity(TextColor);
		BlasterHUD->CharacterOverlay->GameTimerText->SetText(FText::FromString(CountdownText));
	}
}

void ABPlayerController::SetHUDAnnouncementTimer(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Announcement &&
		BlasterHUD->Announcement->WarmupTime &&
		BlasterHUD->Announcement->Blink;
	if(bHUDValid)
	{
		UWidgetAnimation* BlinkAnim = BlasterHUD->Announcement->Blink;
		if(CountdownTime < 5.f)
		{
			if(!BlasterHUD->Announcement->IsAnimationPlaying(BlinkAnim))
			{
				BlasterHUD->Announcement->PlayAnimation(BlinkAnim, 0.f, 0.f,
					EUMGSequencePlayMode::Forward, 1.5f, true);
			}
		}
		
		if(CountdownTime <=  0.f)
		{
			if(HasAuthority())
			{
				BlasterHUD->Announcement->StopAnimation(BlinkAnim);
			}
			BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(""));
			return;
		}
		int32 Seconds = FMath::FloorToInt(CountdownTime);
		FString CountdownText = FString::Printf(TEXT("%02d"), Seconds); 
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
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

void ABPlayerController::SetHUDGrenadesNumber(int32 Grenades, const EBGrenadeCategory GrenadeCategory)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->LethalGrenadesNumber &&
		BlasterHUD->CharacterOverlay->TacticalGrenadesNumber;
		
	if(bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);

		switch (GrenadeCategory)
		{
		case EBGrenadeCategory::EGC_Lethal:
			BlasterHUD->CharacterOverlay->LethalGrenadesNumber->SetText(FText::FromString(GrenadesText));
			break;
		case EBGrenadeCategory::EGC_Tactical:
			BlasterHUD->CharacterOverlay->TacticalGrenadesNumber->SetText(FText::FromString(GrenadesText));
			break;
		default:
			break;
		}
	}
	else
	{
		bInitGrenades = true;
		switch (GrenadeCategory)
		{
		case EBGrenadeCategory::EGC_Lethal:
			HUDLethalGrenades = Grenades;
			break;
		case EBGrenadeCategory::EGC_Tactical:
			HUDTacticalGrenades = Grenades;
			break;
		default:
			break;
		}
	}
}

void ABPlayerController::SetHUDGrenadesImage(UTexture2D* GrenadeImage, const EBGrenadeCategory GrenadeCategory)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->LethalGrenadeImage &&
		BlasterHUD->CharacterOverlay->TacticalGrenadeImage;
		
	if(bHUDValid)
	{
		switch (GrenadeCategory)
		{
		case EBGrenadeCategory::EGC_Lethal:
			BlasterHUD->CharacterOverlay->LethalGrenadeImage->SetBrushFromTexture(GrenadeImage);
			break;
		case EBGrenadeCategory::EGC_Tactical:
			BlasterHUD->CharacterOverlay->TacticalGrenadeImage->SetBrushFromTexture(GrenadeImage);
			break;
		default:
			break;
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

void ABPlayerController::SetGameTime()
{
	float TimeLeft = 0.f;
	if(MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTimeSeconds() + LevelStartedTime;
	}
	else if(MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTimeSeconds() + LevelStartedTime;
	}
	else if (MatchState == MatchState::Cooldown)
	{
		TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTimeSeconds() + LevelStartedTime;
	}
	//uint32 SecondLeft = FMath::CeilToInt(TimeLeft);
	int32 SecondLeft = FMath::CeilToInt(TimeLeft);
	
	if(HasAuthority())
	{
		BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if(BlasterGameMode)
		{
			SecondLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime()) + LevelStartedTime;
		}
	}

	if(CountDown != SecondLeft)
	{
		if(MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementTimer(SecondLeft);
		}
		else if(MatchState == MatchState::InProgress)
		{
			SetHUDGameTimer(SecondLeft);
		}
	}
	CountDown = SecondLeft;
}

void ABPlayerController::StartHighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation;
		
	if(bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation, 0.f,
			5.f);
	}
}

void ABPlayerController::StopHighPingWarning()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation;
		
	if(bHUDValid)
	{
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);

		if(BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation))
		{
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation);
		}
	}
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

void ABPlayerController::ServerCheckMatchState_Implementation()
{
	BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
	if(BlasterGameMode)
	{
		WarmupTime = BlasterGameMode->WarmupTime;
		MatchTime = BlasterGameMode->MatchTime;
		LevelStartedTime = BlasterGameMode->LevelStartedTime;
		CooldownTime = BlasterGameMode->CooldownTime;
		MatchState = BlasterGameMode->GetMatchState();

		if(BlasterHUD && MatchState == MatchState::WaitingToStart)
		{
			BlasterHUD->AddAnnouncement();
		}
		
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, LevelStartedTime, CooldownTime);
	}
}

void ABPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match,
	float LevelStartingTime, float Cooldown)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartedTime = LevelStartingTime;
	MatchState = StateOfMatch;
	CooldownTime = Cooldown;

	// In case this happens after variable replication already happened so the proper values are used
	OnMatchStateSet(MatchState);

	if(BlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnouncement();
	}
}

void ABPlayerController::ClientSetHUDPlayerStats_Implementation(const TArray<FPlayerStats>& PlayerStats)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Scoreboard;
		
	LocalPlayerStats = PlayerStats;
	
	if(bHUDValid)
	{
		for(FPlayerStats Stats : PlayerStats)
		{
			BlasterHUD->Scoreboard->UpdatePlayerList(PlayerStats);
		}
	}
	else
	{
		bInitScoreboard = true;
	}
}

void ABPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD)
	{
		if(BlasterHUD->bHUDAdded == false)
		{
			BlasterHUD->AddHUD();
		}
		if(BlasterHUD->Announcement && BlasterHUD->Announcement->Blink)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Collapsed);
			BlasterHUD->Announcement->StopAnimation(BlasterHUD->Announcement->Blink);
			for(FPlayerStats Stats : LocalPlayerStats)
			{
				BlasterHUD->Scoreboard->UpdatePlayerList(LocalPlayerStats);
			}
		}
	}
}

void ABPlayerController::HandleCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD)
	{
		BlasterHUD->RemoveHUD(true, false, true);
		if(BlasterHUD->Announcement && BlasterHUD->Announcement->NewGameText &&  BlasterHUD->Announcement->InfoText)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			BlasterHUD->Announcement->NewGameText->SetVisibility(ESlateVisibility::Visible);

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(GetWorld()));
			ABPlayerState* PS = GetPlayerState<ABPlayerState>();
			if(BlasterGameState && PS)
			{
				TArray<ABPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
				FString WinnerInfo;
				
				if(TopPlayers.IsEmpty())
				{
					WinnerInfo = FString("Not a soul was killed, how disappointing");
				}
				else if(TopPlayers.Num() == 1 && TopPlayers[0] == PS)
				{
					WinnerInfo = FString("You are the winner, good job soldier");
				}
				else if(TopPlayers.Num() == 1)
				{
					WinnerInfo = FString::Printf(TEXT("Winner is: \n%s \n shame on the others"),
						*TopPlayers[0]->GetPlayerName());
				}
				else if(TopPlayers.Num() > 1)
				{
					WinnerInfo = FString("Players equally cool: \n");
					for(auto TiedPlayer : TopPlayers)
					{
						WinnerInfo.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}
				
				BlasterHUD->Announcement->InfoText->SetVisibility(ESlateVisibility::Visible);
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(WinnerInfo));
			}
		}
	}

	if(GEngine)
	{
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GEngine->GetFirstGamePlayer(GetWorld()));
		if(InputSubsystem)
		{
			if(ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetCharacter()))
			{
				TArray<UInputMappingContext*> MappingContexts = BlasterCharacter->GetGameplayMappingContexts();
				for(const UInputMappingContext* Context : MappingContexts)
				{
					InputSubsystem->RemoveMappingContext(Context);
				}

				if(BlasterCharacter->GetCombatComp())
				{
					BlasterCharacter->GetCombatComp()->FireButtonPressed(false);
					BlasterCharacter->bDisableGameplay = true;
				}
			}
		}
	}
}

void ABPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}




