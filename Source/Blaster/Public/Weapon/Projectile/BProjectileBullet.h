// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/BProjectile.h"
#include "BProjectileBullet.generated.h"

class UBProjectileMovementComponent;

/**
 * 
 */
UCLASS()
class BLASTER_API ABProjectileBullet : public ABProjectile
{
	GENERATED_BODY()

public:
	ABProjectileBullet();

protected:
	virtual void BeginPlay() override;
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit) override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBProjectileMovementComponent> BProjectileMoveComp;
	
};
