// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BInGameMenu.h"
#include "Components/Button.h"
#include "GameFramework/PlayerController.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "Player/BPlayerController.h"

void UBInGameMenu::InGameMenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
	
	PC =  PC == nullptr ? GetOwningPlayer() : PC;
	if(PC)
	{
		FInputModeGameAndUI InputModeData;
		InputModeData.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputModeData);
		PC->SetShowMouseCursor(true);
	}

	if(ReturnButton && !ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.AddDynamic(this, &ThisClass::ReturnButtonClicked);
	}
	
	if(MenuButton && !MenuButton->OnClicked.IsBound())
	{
		MenuButton->OnClicked.AddDynamic(this, &ThisClass::MenuButtonClicked);
	}

	if(OptionsButton && !OptionsButton->OnClicked.IsBound())
	{
		OptionsButton->OnClicked.AddDynamic(this, &ThisClass::OptionsButtonClicked);
	}
	
	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if(MultiplayerSessionsSubsystem && !MultiplayerSessionsSubsystem->MultiplayerSystemOnDestroySessionComplete.IsBound())
		{
			MultiplayerSessionsSubsystem->MultiplayerSystemOnDestroySessionComplete.AddDynamic(this,
				&ThisClass::OnDestroySession);
		}
	}
}

void UBInGameMenu::InGameMenuTearDown()
{
	RemoveFromParent();

	PC =  PC == nullptr ? GetOwningPlayer() : PC;
	if(PC)
	{
		FInputModeGameOnly InputModeData;
		PC->SetInputMode(InputModeData);
		PC->SetShowMouseCursor(false);
	}

	if(ReturnButton && ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.RemoveDynamic(this, &ThisClass::ReturnButtonClicked);
	}
	
	if(MenuButton && MenuButton->OnClicked.IsBound())
	{
		MenuButton->OnClicked.RemoveDynamic(this, &ThisClass::MenuButtonClicked);
	}

	if(OptionsButton && OptionsButton->OnClicked.IsBound())
	{
		OptionsButton->OnClicked.RemoveDynamic(this, &ThisClass::OptionsButtonClicked);
	}

	if(MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerSystemOnDestroySessionComplete.IsBound())
	{
		MultiplayerSessionsSubsystem->MultiplayerSystemOnDestroySessionComplete.RemoveDynamic(this,
			&ThisClass::OnDestroySession);
	}
}

bool UBInGameMenu::Initialize()
{
	if(!Super::Initialize())
	{
		return false;
	}
	return true;
}

void UBInGameMenu::NativeDestruct()
{
	Super::NativeDestruct();
}

void UBInGameMenu::OnDestroySession(bool bWasSuccessful)
{
	if(!bWasSuccessful)
	{
		ReturnButton->SetIsEnabled(true);
	}
	
	UWorld* World = GetWorld();
	if(World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if(GameMode)
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			PC = PC == nullptr ? GetOwningPlayer() : PC;
			if(PC)
			{
				PC->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}

void UBInGameMenu::ReturnButtonClicked()
{
	PC = PC == nullptr ? GetOwningPlayer() : PC;
	if(ABPlayerController* BPC = Cast<ABPlayerController>(PC))
	{
		BPC->ReturnFromInGameMenu();
	}
}

void UBInGameMenu::MenuButtonClicked()
{
	ReturnButton->SetIsEnabled(false);
	
	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();
	}
	else
	{
		ReturnButton->SetIsEnabled(true);
	}
}

void UBInGameMenu::OptionsButtonClicked()
{
}
