// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/BProjectile.h"
#include "BProjectileRocket.generated.h"

class UStaticMeshComponent;
/**
 * 
 */
UCLASS()
class BLASTER_API ABProjectileRocket : public ABProjectile
{
	GENERATED_BODY()

public:
	ABProjectileRocket();
	
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit) override;

protected:
	// Minimal damage applied to actors that are in the outer ring of radial damage
	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile|Damage")
	float MinimalDamage = 10.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile")
	float DamageInnerRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile")
	float DamageOuterRadius = 400.f;

private:
	UPROPERTY(VisibleAnywhere, Category = "Blaster")
	TObjectPtr<UStaticMeshComponent> RocketMesh;
};
