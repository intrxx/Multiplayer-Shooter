// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BCharacterOverlay.generated.h"

class UProgressBar;
class UTextBlock;
class UOverlay;

/**
 * 
 */
UCLASS()
class BLASTER_API UBCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HealthText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KilledByName;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageDone;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageTaken;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> DeathInfoOverlay;
};
