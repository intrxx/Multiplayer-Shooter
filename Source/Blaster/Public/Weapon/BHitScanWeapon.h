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

protected:
	void HitScanTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

protected:
	UPROPERTY(EditAnywhere, Category = "Blaster|HitScan")
	float Damage = 20.f;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|HitScan")
	TObjectPtr<UParticleSystem> SurfaceImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Blaster|HitScan")
	TObjectPtr<UParticleSystem> CharacterImpactParticles;

	UPROPERTY(EditAnywhere, Category = "Blaster|HitScan")
	TObjectPtr<UParticleSystem> BeamParticles;

	UPROPERTY(EditAnywhere, Category = "Blaster|HitScan")
	TObjectPtr<USoundCue> CharacterHitSound;

	UPROPERTY(EditAnywhere, Category = "Blaster|HitScan")
	TObjectPtr<USoundCue> SurfaceHitSound;

	UPROPERTY(EditAnywhere, Category = "Blaster|HitScan|Additional")
	TObjectPtr<UParticleSystem> MuzzleFlash;

	UPROPERTY(EditAnywhere, Category = "Blaster|HitScan|Additional")
	TObjectPtr<USoundCue> FireSound;

private:
	
};
