// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BKillFeed.generated.h"

class UHorizontalBox;
class UImage;
class UTextBlock;

/**
 * 
 */
UCLASS()
class BLASTER_API UBKillFeed : public UUserWidget
{
	GENERATED_BODY()

public:

	void SetKillFeedText(const FString& KillerName, const FString& KilledName, UTexture2D* GunTextureImage);
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> KillFeedBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KillerPlayerName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KilledPlayerName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> GunImage;
};
