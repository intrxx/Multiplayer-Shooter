// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BScoreBoard.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class BLASTER_API UBScoreBoard : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreAmount;

	bool bIsVisible = false;
};
