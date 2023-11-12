// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUD/BlasterHUD.h"
#include "BScoreboardPlayerEntry.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class BLASTER_API UBScoreboardPlayerEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetPlayerStats(FPlayerStats PlayerStats);
	
public:	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KillsAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DeathsAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AssistsAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageAmount;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScoreAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PingAmount;

	FPlayerStats PlayerStats;
};
