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
	virtual void ShotgunScatter(const FVector& HitTarget, /* OUT */ TArray<FVector>& OutHitTargets);
private:
	UPROPERTY(EditAnywhere, Category = "Blaster|Shotgun")
	uint32 NumberOfPallets = 10;
};
