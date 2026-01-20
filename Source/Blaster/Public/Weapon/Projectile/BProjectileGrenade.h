// Copyright 2026 out of sCope team - intrxx

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/BProjectile.h"
#include "BProjectileGrenade.generated.h"

class UBProjectileMovementComponent;

/**
 * 
 */
UCLASS()
class BLASTER_API ABProjectileGrenade : public ABProjectile
{
	GENERATED_BODY()

public:
	ABProjectileGrenade();

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
	
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBProjectileMovementComponent> BProjectileMoveComp;

	UPROPERTY(EditAnywhere, Category = "Blaster|Grenade")
	TObjectPtr<USoundCue> BounceSound;

	UPROPERTY(EditAnywhere, Category = "Blaster|Grenade")
	bool bShouldExplodeOnEnemy = true;
	
};
