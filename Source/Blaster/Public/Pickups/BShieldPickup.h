// Copyright 2026 out of sCope team - intrxx

#pragma once

#include "CoreMinimal.h"
#include "Pickups/BPickup.h"
#include "BShieldPickup.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABShieldPickup : public ABPickup
{
	GENERATED_BODY()

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere, Category = "Blaster|ShieldPickup")
	float DefaultShieldReplenishAmount = 50.f;
};
