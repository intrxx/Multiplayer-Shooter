// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BlasterHUD.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "HUD/BCharacterOverlay.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();

	AddCharacterOverlay();
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
