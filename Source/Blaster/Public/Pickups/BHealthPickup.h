// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/BPickup.h"
#include "BHealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABHealthPickup : public ABPickup
{
	GENERATED_BODY()

public:
	ABHealthPickup();

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere, Category = "Blaster|HealthPickup")
	float HealAmount = 50.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|HealthPickup")
	float HealingTime = 3.f;
};
