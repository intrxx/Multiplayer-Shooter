// Copyright 2026 out of sCope team - intrxx

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
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere, Category = "Blaster|HealthPickup")
	float HealAmount = 50.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|HealthPickup")
	float HealingTime = 3.f;
};
