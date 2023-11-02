// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if(GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if(HUDPackage.CrosshairDot && CrosshairInfo.bDrawCrosshairDot == true)
		{
			DrawCrosshairElement(HUDPackage.CrosshairDot, ViewportCenter, FVector2D::Zero());
		}
		
		if(HUDPackage.CrosshairBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshairElement(HUDPackage.CrosshairBottom, ViewportCenter, Spread);
		}

		if(HUDPackage.CrosshairTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshairElement(HUDPackage.CrosshairTop, ViewportCenter, Spread);
		}

		if(HUDPackage.CrosshairLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshairElement(HUDPackage.CrosshairLeft, ViewportCenter, Spread);
		}

		if(HUDPackage.CrosshairRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshairElement(HUDPackage.CrosshairRight, ViewportCenter, Spread);
		}
	}
}

void ABlasterHUD::DrawCrosshairElement(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y);
	
	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight,
		0.f, 0.f, 1.f, 1.f, FLinearColor::White);
}
