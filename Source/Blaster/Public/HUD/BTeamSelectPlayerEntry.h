// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BTeamSelectPlayerEntry.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class BLASTER_API UBTeamSelectPlayerEntry : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetName(const FString& Name);
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PlayerName;
};
