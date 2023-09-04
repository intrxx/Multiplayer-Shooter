// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OverheadWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetNetRoleText(FString TextToDisplay)
{
	if(NetRoleText)
	{
		NetRoleText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	if(!InPawn)
	{
		return;
	}
	ENetRole RemoteRole = InPawn->GetRemoteRole();

	FString Role;
	switch (RemoteRole)
	{
	case ROLE_Authority:
		Role = FString("Authority");
		break;
	case ROLE_AutonomousProxy:
		Role = FString("AutonomousProxy");
		break;
	case ROLE_SimulatedProxy:
		Role = FString("SimulatedProxy");
		break;
	case ROLE_None:
		Role = FString("None");
		break;
	default:
		Role = FString("None");
		break;
	}
	
	APlayerState* PS = InPawn->GetPlayerState();
	if(PS)
	{
		FString PlayerName = PS->GetPlayerName();
		
		FString RemoteRoleString = FString::Printf(TEXT("%s's Remote Role: %s"),*PlayerName, *Role);
		SetNetRoleText(RemoteRoleString);
	}
	else
	{
		FString RemoteRoleString = FString::Printf(TEXT("Remote Role: %s"), *Role);
		SetNetRoleText(RemoteRoleString);
	}
	
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	
	Super::NativeDestruct();
}
