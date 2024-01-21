// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BKillFeed.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UBKillFeed::SetKillFeedText(const FString& KillerName, const FString& KilledName, UTexture2D* GunTextureImage)
{
	if(KillerPlayerName && KilledPlayerName)
	{
		KillerPlayerName->SetText(FText::FromString(KillerName));
		KilledPlayerName->SetText(FText::FromString(KilledName));
	}
	
	if(GunImage)
	{
		GunImage->SetBrushFromTexture(GunTextureImage);
	}
}
