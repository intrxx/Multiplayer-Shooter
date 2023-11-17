// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BlasterHUD.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "HUD/BCharacterOverlay.h"
#include "HUD/BScoreBoard.h"
#include "HUD/BInventoryWidget.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();
	
	AddCharacterOverlay();
	AddScoreBoard();
	AddInventoryWidget();
}

void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PC = GetOwningPlayerController();
	if(PC && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UBCharacterOverlay>(PC, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::AddScoreBoard()
{
	APlayerController* PC = GetOwningPlayerController();
	if(PC && ScoreboardClass)
	{
		Scoreboard = CreateWidget<UBScoreBoard>(PC, ScoreboardClass);
		Scoreboard->AddToViewport();
		Scoreboard->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ABlasterHUD::AddInventoryWidget()
{
	APlayerController* PC = GetOwningPlayerController();
	if(PC && InventoryClass)
	{
		Inventory = CreateWidget<UBInventoryWidget>(PC, InventoryClass);
		Inventory->AddToViewport();
		Inventory->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ABlasterHUD::ToggleScoreboard(bool bIsVisible)
{
	if(Scoreboard == nullptr)
	{
		return;
	}

	if(bIsVisible)
	{
		Scoreboard->SetVisibility(ESlateVisibility::Collapsed);
		Scoreboard->bIsVisible = false;
	}
	else
	{
		Scoreboard->SetVisibility(ESlateVisibility::Visible);
		Scoreboard->bIsVisible = true;
	}
}

void ABlasterHUD::ToggleInventory(bool bIsVisible)
{
	if(Inventory == nullptr)
	{
		return;
	}

	if(bIsVisible)
	{
		Inventory->SetVisibility(ESlateVisibility::Collapsed);
		Inventory->bIsVisible = false;
	}
	else
	{
		Inventory->SetVisibility(ESlateVisibility::Visible);
		Inventory->bIsVisible = true;
	}
}

bool ABlasterHUD::IsScoreboardVisible()
{
	if(Scoreboard)
	{
		return Scoreboard->bIsVisible;
	}
	return false;
}

bool ABlasterHUD::IsInventoryVisible()
{
	if(Inventory)
	{
		return Inventory->bIsVisible;
	}
	return false;
}

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if(GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * CrosshairInfo.CrosshairSpread;

		if(CrosshairInfo.CrosshairDot && CrosshairInfo.bDrawCrosshairDot)
		{
			DrawCrosshairElement(CrosshairInfo.CrosshairDot, ViewportCenter, FVector2D::ZeroVector,
				CrosshairInfo.CrosshairColor);
		}
		
		if(CrosshairInfo.CrosshairBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshairElement(CrosshairInfo.CrosshairBottom, ViewportCenter, Spread, CrosshairInfo.CrosshairColor);
		}

		if(CrosshairInfo.CrosshairTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshairElement(CrosshairInfo.CrosshairTop, ViewportCenter, Spread, CrosshairInfo.CrosshairColor);
		}

		if(CrosshairInfo.CrosshairLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshairElement(CrosshairInfo.CrosshairLeft, ViewportCenter, Spread, CrosshairInfo.CrosshairColor);
		}

		if(CrosshairInfo.CrosshairRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshairElement(CrosshairInfo.CrosshairRight, ViewportCenter, Spread, CrosshairInfo.CrosshairColor);
		}
	}
}

void ABlasterHUD::DrawCrosshairElement(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y);
	
	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight,
		0.f, 0.f, 1.f, 1.f, CrosshairColor);
}
