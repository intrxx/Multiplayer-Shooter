// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BAnnouncement.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class BLASTER_API UBAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WarmupTime;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> InfoText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NewGameText;
	
};
