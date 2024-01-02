// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/BPickup.h"
#include "BJumpPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABJumpPickup : public ABPickup
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere, Category = "Blaster|JumpBuff")
	float JumpZVelocityBuff = 4000.f;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|JumpBuff")
	float JumpBuffTime = 15.f;
};
