// Copyright 2026 out of sCope team - intrxx

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BWeapon.h"
#include "BProjectileWeapon.generated.h"

class ABProjectile;
/**
 * 
 */
UCLASS()
class BLASTER_API ABProjectileWeapon : public ABWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	TSubclassOf<ABProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	TSubclassOf<ABProjectile> ServerSideRewindProjectileClass;
};
