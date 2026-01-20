// Copyright 2026 out of sCope team - intrxx

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "BRocketProjectileMovementComp.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBRocketProjectileMovementComp : public UProjectileMovementComponent
{
	GENERATED_BODY()
	
protected:
	virtual EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining) override;
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta) override;
};
