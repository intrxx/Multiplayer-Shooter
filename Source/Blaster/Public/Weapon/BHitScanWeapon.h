// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BWeapon.h"
#include "BHitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABHitScanWeapon : public ABWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere, Category = "Blaster|HitScan")
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|HitScan")
	TObjectPtr<UParticleSystem> SurfaceImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Blaster|HitScan")
	TObjectPtr<UParticleSystem> CharacterImpactParticles;
};
