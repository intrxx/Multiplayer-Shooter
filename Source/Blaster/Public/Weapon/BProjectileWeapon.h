// Fill out your copyright notice in the Description page of Project Settings.

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
	ABProjectileWeapon();

	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	TSubclassOf<ABProjectile> ProjectileClass;
};
