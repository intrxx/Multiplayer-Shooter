// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BCharacterOverlay.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;
class UOverlay;
class UTexture2D;

/**
 * 
 */
UCLASS()
class BLASTER_API UBCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * Main Overlay
	 */
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HealthText;
	
	/**
	 *  Death Overlay
	 */
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> DeathInfoOverlay;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KilledByName;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageDone;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageTaken;

	/**
	 * Ammo and Weapon
	 */
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> AmmoImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WeaponAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CarriedAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WeaponType;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Ammo")
	TObjectPtr<UTexture2D> SingleAmmoImage;

	UPROPERTY(EditAnywhere, Category = "Blaster|Ammo")
	TObjectPtr<UTexture2D> BurstAmmoImage;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Ammo")
	TObjectPtr<UTexture2D> AutomaticAmmoImage;
};
