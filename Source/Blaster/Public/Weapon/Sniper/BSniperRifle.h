// Copyright 2026 out of sCope team - intrxx

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BHitScanWeapon.h"
#include "BSniperRifle.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABSniperRifle : public ABHitScanWeapon
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Blaster|Sniper")
	TObjectPtr<USoundCue> ScopeInSound;

	UPROPERTY(EditAnywhere, Category = "Blaster|Sniper")
	TObjectPtr<USoundCue> ScopeOutSound;
};
