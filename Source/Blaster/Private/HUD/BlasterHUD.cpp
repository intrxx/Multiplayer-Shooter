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

		if(HUDPackage.CrosshairDot && CrosshairInfo.bDrawCrosshairDot == true)
		{
			DrawCrosshairElement(HUDPackage.CrosshairDot, ViewportCenter);
		}
		
		if(HUDPackage.CrosshairBottom)
		{
			DrawCrosshairElement(HUDPackage.CrosshairBottom, ViewportCenter);
		}

		if(HUDPackage.CrosshairTop)
		{
			DrawCrosshairElement(HUDPackage.CrosshairTop, ViewportCenter);
		}

		if(HUDPackage.CrosshairLeft)
		{
			DrawCrosshairElement(HUDPackage.CrosshairLeft, ViewportCenter);
		}

		if(HUDPackage.CrosshairRight)
		{
			DrawCrosshairElement(HUDPackage.CrosshairRight, ViewportCenter);
		}
	}
}

void ABlasterHUD::DrawCrosshairElement(UTexture2D* Texture, FVector2D ViewportCenter)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f),
		ViewportCenter.Y - (TextureHeight / 2.f));
	
	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight,
		0.f, 0.f, 1.f, 1.f, FLinearColor::White);
}
