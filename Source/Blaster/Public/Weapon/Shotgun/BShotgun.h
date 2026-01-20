// Copyright 2026 out of sCope team - intrxx

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
	virtual void FireShotgun(const TArray<FVector_NetQuantize>& HitTargets);
	virtual void ShotgunScatter(const FVector& HitTarget, /* OUT */ TArray<FVector_NetQuantize>& OutHitTargets);
private:
	UPROPERTY(EditAnywhere, Category = "Blaster|Shotgun")
	uint32 NumberOfPallets = 10;
};
