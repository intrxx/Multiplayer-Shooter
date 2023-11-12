// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HUD/BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "BPlayerController.generated.h"


class ABlasterHUD;

/**
 * 
 */
UCLASS()
class BLASTER_API ABPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	
	void SetHUDPlayerNames(TArray<FPlayerStats> PlayerStats);
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	TObjectPtr<ABlasterHUD> BlasterHUD;
	
};
