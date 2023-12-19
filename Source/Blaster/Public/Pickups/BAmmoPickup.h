// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/BPickup.h"
#include "BlasterTypes/BWeaponTypes.h"
#include "BAmmoPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABAmmoPickup : public ABPickup
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere, Category = "Blaster|Ammo")
	int32 AmmoAmount = 30;

	UPROPERTY(EditAnywhere, Category = "Blaster|Ammo")
	EBWeaponType WeaponType;
};
