// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BInventoryWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class BLASTER_API UBInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SniperWeaponAmmoAmount;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RifleWeaponAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ShotgunWeaponAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PistolWeaponAmmoAmount;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RocketWeaponAmmoAmount;

	bool bIsVisible = false;
	
};
