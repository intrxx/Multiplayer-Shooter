// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BTeamSelect.h"
#include "Components/Button.h"
#include "Player/BPlayerState.h"

void UBTeamSelect::TeamSelectSetup()
{
	SetIsFocusable(true);
	
	PC =  PC == nullptr ? GetOwningPlayer() : PC;
	if(PC)
	{
		FInputModeGameAndUI InputModeData;
		InputModeData.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputModeData);
		PC->SetShowMouseCursor(true);
	}
	
	if(RedTeamButton && !RedTeamButton->OnClicked.IsBound())
	{
		RedTeamButton->OnClicked.AddDynamic(this, &ThisClass::OnRedSelectClicked);
	}

	if(BlueTeamButton && !BlueTeamButton->OnClicked.IsBound())
	{
		BlueTeamButton->OnClicked.AddDynamic(this, &ThisClass::OnBlueSelectClicked);
	}
}

void UBTeamSelect::TeamSelectTearDown()
{
	RemoveFromParent();

	PC =  PC == nullptr ? GetOwningPlayer() : PC;
	if(PC)
	{
		FInputModeGameOnly InputModeData;
		PC->SetInputMode(InputModeData);
		PC->SetShowMouseCursor(false);
	}

	if(RedTeamButton && RedTeamButton->OnClicked.IsBound())
	{
		RedTeamButton->OnClicked.RemoveDynamic(this, &ThisClass::OnRedSelectClicked);
	}
	
	if(BlueTeamButton && BlueTeamButton->OnClicked.IsBound())
	{
		BlueTeamButton->OnClicked.RemoveDynamic(this, &ThisClass::OnBlueSelectClicked);
	}
}

void UBTeamSelect::OnRedSelectClicked()
{
	ABPlayerState* BPS = Cast<ABPlayerState>(GetOwningPlayerState());
	if(BPS)
	{
		BPS->ServerSetTeam(EBTeam::EBT_RedTeam);
	}
	TeamSelectTearDown();
}

void UBTeamSelect::OnBlueSelectClicked()
{
	ABPlayerState* BPS = Cast<ABPlayerState>(GetOwningPlayerState());
	if(BPS)
	{
		BPS->ServerSetTeam(EBTeam::EBT_BlueTeam);
	}
	TeamSelectTearDown();
}

