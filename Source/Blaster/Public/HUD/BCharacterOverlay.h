// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BCharacterOverlay.generated.h"

class USizeBox;
class UImage;
class UProgressBar;
class UTextBlock;
class UOverlay;
class UTexture2D;
class UWidgetAnimation;

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

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ShieldProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ShieldText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GameTimerText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> HighPingImage;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> HighPingAnimation;
	
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

	/**
	 * Grenades
	 */

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> LethalGrenadeImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LethalGrenadesNumber;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> TacticalGrenadeImage;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TacticalGrenadesNumber;

	/**
	 * Leading player
	 */

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LeadingPlayerName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LeadingPlayerKillsText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> LeadingPlayerBox;
	
};
