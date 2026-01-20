// Copyright 2026 out of sCope team - intrxx


#include "BlasterComponents/BRocketProjectileMovementComp.h"

UProjectileMovementComponent::EHandleBlockingHitResult UBRocketProjectileMovementComp::HandleBlockingHit(
	const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void UBRocketProjectileMovementComp::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	// Rockets should not stop; only explode when their CollisionBox detects a hit
}
