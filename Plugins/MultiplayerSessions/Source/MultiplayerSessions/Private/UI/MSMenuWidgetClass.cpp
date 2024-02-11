// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MSMenuWidgetClass.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"

void UMSMenuWidgetClass::MenuSetup(int32 NumOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumOfPublicConnections;
	MatchType = TypeOfMatch;
	
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
	
	if(UWorld* World = GetWorld())
	{
		if(APlayerController* PC = World->GetFirstPlayerController())
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			
			PC->SetInputMode(InputModeData);

			if(!bIsCursorEnabledInGame)
			{
				PC->SetShowMouseCursor(true);
			}
		}
	}
	
	if(UGameInstance* GameInstance = GetGameInstance())
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerSystemOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerSystemOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerSystemOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);

		MultiplayerSessionsSubsystem->MultiplayerSystemOnFindSessionComplete.AddUObject(this, &ThisClass::OnFindSession);
		MultiplayerSessionsSubsystem->MultiplayerSystemOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
	}
}

bool UMSMenuWidgetClass::Initialize()
{
	if(!Super::Initialize())
	{
		return false;
	}

	if(HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}

	if(JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}

	if(ShowGameModesButton)
	{
		ShowGameModesButton->OnClicked.AddDynamic(this, &ThisClass::ModesButtonClicked);
	}

	if(CaptureTheFlagCheckBox)
	{
		CaptureTheFlagCheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::CaptureTheFlagCheckStateChanged);
	}

	if(FFADeathMatchCheckBox)
	{
		FFADeathMatchCheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::FFADeathMatchCheckStateChanged);
	}

	if(TeamDeathMatchCheckBox)
	{
		TeamDeathMatchCheckBox->OnCheckStateChanged.AddDynamic(this, &ThisClass::TeamDeathMatchCheckStateChanged);
	}

	if(NumOfPlayerTextBox)
	{
		NumOfPlayerTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::PlayersCountTextCommitted);
	}
	
	return true;
}

void UMSMenuWidgetClass::NativeDestruct()
{
	MenuTearDown();
	
	Super::NativeDestruct();
}

void UMSMenuWidgetClass::OnCreateSession(bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		UWorld* World = GetWorld();
		if(World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else
	{
		HostButton->SetIsEnabled(true);
	}
}

void UMSMenuWidgetClass::OnDestroySession(bool bWasSuccessful)
{
}

void UMSMenuWidgetClass::OnStartSession(bool bWasSuccessful)
{
	if(MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}
}

void UMSMenuWidgetClass::OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if(MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}
	
	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);

		if(SettingsValue == MatchType)
		{
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}

	if(!bWasSuccessful || SessionResults.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMSMenuWidgetClass::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if(Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if(SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController();
			if(PC)
			{
				PC->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}

	if(Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMSMenuWidgetClass::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);

	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMSMenuWidgetClass::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	
	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(20000);
	}
}

void UMSMenuWidgetClass::ModesButtonClicked()
{
	bool bModesUIValid = ModesBox && CaptureTheFlagCheckBox && TeamDeathMatchCheckBox && FFADeathMatchCheckBox;
	
	if(bModesUIValid)
	{
		if(bIsModesVisible)
		{
			bIsModesVisible = false;
			PlayAnimation(Hide);
			CaptureTheFlagCheckBox->SetIsEnabled(false);
			TeamDeathMatchCheckBox->SetIsEnabled(false);
			FFADeathMatchCheckBox->SetIsEnabled(false);
		}
		else
		{
			bIsModesVisible = true;
			PlayAnimation(Appear);
			FFADeathMatchCheckBox->SetIsEnabled(true);
			TeamDeathMatchCheckBox->SetIsEnabled(true);
			CaptureTheFlagCheckBox->SetIsEnabled(true);
		}
	}
}

void UMSMenuWidgetClass::FFADeathMatchCheckStateChanged(bool bIsChecked)
{
	if(bIsChecked)
	{
		if(TeamDeathMatchCheckBox)
		{
			TeamDeathMatchCheckBox->SetCheckedState(ECheckBoxState::Unchecked);
		}
		if(CaptureTheFlagCheckBox)
		{
			CaptureTheFlagCheckBox->SetCheckedState(ECheckBoxState::Unchecked);
		}

		MatchType = FString(TEXT("FreeForAll"));
	}
}

void UMSMenuWidgetClass::TeamDeathMatchCheckStateChanged(bool bIsChecked)
{
	if(bIsChecked)
	{
		if(FFADeathMatchCheckBox)
		{
			FFADeathMatchCheckBox->SetCheckedState(ECheckBoxState::Unchecked);
		}
		if(CaptureTheFlagCheckBox)
		{
			CaptureTheFlagCheckBox->SetCheckedState(ECheckBoxState::Unchecked);
		}

		MatchType = FString(TEXT("TeamDeathMatch"));
	}
}

void UMSMenuWidgetClass::CaptureTheFlagCheckStateChanged(bool bIsChecked)
{
	if(bIsChecked)
	{
		if(TeamDeathMatchCheckBox)
		{
			TeamDeathMatchCheckBox->SetCheckedState(ECheckBoxState::Unchecked);
		}
		if(FFADeathMatchCheckBox)
		{
			FFADeathMatchCheckBox->SetCheckedState(ECheckBoxState::Unchecked);
		}
		
		MatchType = FString(TEXT("CaptureTheFlag"));
	}
}

void UMSMenuWidgetClass::PlayersCountTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	const int32 NumOfPlayers = FCString::Atoi(*Text.ToString());
	
	NumPublicConnections = NumOfPlayers;
}

void UMSMenuWidgetClass::MenuTearDown()
{
	RemoveFromParent();
	
	if(UWorld* World = GetWorld())
	{
		if(APlayerController* PC  = World->GetFirstPlayerController())
		{
			FInputModeGameOnly InputModeData;
			PC->SetInputMode(InputModeData);

			PC->SetShowMouseCursor(bIsCursorEnabledInGame);
		}
	}
}
