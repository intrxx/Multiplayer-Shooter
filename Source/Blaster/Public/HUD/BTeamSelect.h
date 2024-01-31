// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BTeamSelect.generated.h"

class ABlasterCharacter;
class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class BLASTER_API UBTeamSelect : public UUserWidget
{
	GENERATED_BODY()
public:
	void TeamSelectSetup();
	void TeamSelectTearDown();
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RedTeamButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BlueTeamButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> RandomTeamButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RedTeamCounter;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BlueTeamCounter;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RandomCounter;

protected:
	UFUNCTION()
	void OnRedSelectClicked();

	UFUNCTION()
	void OnBlueSelectClicked();

	UFUNCTION()
	void OnRandomSelectClicked();

	UPROPERTY()
	TObjectPtr<APlayerController> PC;
	
};
