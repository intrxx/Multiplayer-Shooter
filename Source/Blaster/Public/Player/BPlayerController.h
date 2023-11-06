// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
protected:
	virtual void BeginPlay() override;
	
private:
	TObjectPtr<ABlasterHUD> BlasterHUD;
};
