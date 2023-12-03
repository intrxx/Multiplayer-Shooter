// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BHitScanWeapon.h"
#include "BShotgun.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABShotgun : public ABHitScanWeapon
{
	GENERATED_BODY()

public:
	void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere, Category = "Blaster|Shotgun")
	uint32 NumberOfPallets = 10;
};
