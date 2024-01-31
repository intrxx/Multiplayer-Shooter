// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BTeamSelect.h"
#include "Character/BlasterCharacter.h"
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

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetOwningPlayerPawn());
	if(BlasterCharacter)
	{
		BlasterCharacter->DisableInput(PC);
	}
	
	if(RedTeamButton && !RedTeamButton->OnClicked.IsBound())
	{
		RedTeamButton->OnClicked.AddDynamic(this, &ThisClass::OnRedSelectClicked);
	}

	if(BlueTeamButton && !BlueTeamButton->OnClicked.IsBound())
	{
		BlueTeamButton->OnClicked.AddDynamic(this, &ThisClass::OnBlueSelectClicked);
	}

	if(RandomTeamButton && !RandomTeamButton->OnClicked.IsBound())
	{
		RandomTeamButton->OnClicked.AddDynamic(this, &ThisClass::OnRandomSelectClicked);
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

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetOwningPlayerPawn());
	if(BlasterCharacter)
	{
		BlasterCharacter->EnableInput(PC);
	}

	if(RedTeamButton && RedTeamButton->OnClicked.IsBound())
	{
		RedTeamButton->OnClicked.RemoveDynamic(this, &ThisClass::OnRedSelectClicked);
	}
	
	if(BlueTeamButton && BlueTeamButton->OnClicked.IsBound())
	{
		BlueTeamButton->OnClicked.RemoveDynamic(this, &ThisClass::OnBlueSelectClicked);
	}

	if(RandomTeamButton && RandomTeamButton->OnClicked.IsBound())
	{
		RandomTeamButton->OnClicked.RemoveDynamic(this, &ThisClass::OnRandomSelectClicked);
	}
}

void UBTeamSelect::OnRedSelectClicked()
{
	ABPlayerState* BPS = Cast<ABPlayerState>(GetOwningPlayerState());
	if(BPS == nullptr)
	{
		return;
	}
	
	if(BPS->GetTeam() != EBTeam::EBT_RedTeam)
	{
		BPS->ServerSetTeam(EBTeam::EBT_RedTeam);
	}
}

void UBTeamSelect::OnBlueSelectClicked()
{
	ABPlayerState* BPS = Cast<ABPlayerState>(GetOwningPlayerState());
	if(BPS == nullptr)
	{
		return;
	}
	
	if(BPS->GetTeam() != EBTeam::EBT_BlueTeam)
	{
		BPS->ServerSetTeam(EBTeam::EBT_BlueTeam);
	}
}

void UBTeamSelect::OnRandomSelectClicked()
{
	ABPlayerState* BPS = Cast<ABPlayerState>(GetOwningPlayerState());
	if(BPS == nullptr)
	{
		return;
	}

	if(BPS->GetTeam() != EBTeam::EBT_ChooseRandomTeam)
	{
		BPS->ServerSetTeam(EBTeam::EBT_ChooseRandomTeam);
	}
}

