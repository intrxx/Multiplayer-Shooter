// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/BPickup.h"
#include "BSpeedPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABSpeedPickup : public ABPickup
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere, Category = "Blaster|SpeedBuff")
	float BaseSpeedBuff = 1300.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|SpeedBuff")
	float CrouchSpeedBuff = 700.f;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|SpeedBuff")
	float SpeedBuffTime = 30.f;

	
};
