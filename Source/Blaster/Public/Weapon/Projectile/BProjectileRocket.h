// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/BProjectile.h"
#include "BProjectileRocket.generated.h"

class UNiagaraComponent;
class UStaticMeshComponent;
class UNiagaraSystem;
class UAudioComponent;
class USoundCue;
class USoundAttenuation;
/**
 * 
 */
UCLASS()
class BLASTER_API ABProjectileRocket : public ABProjectile
{
	GENERATED_BODY()

public:
	ABProjectileRocket();
	virtual void Destroyed() override;
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;

	void DestroyTimerFinished();

protected:
	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile|FX")
	TObjectPtr<UNiagaraSystem> TrailSystem;
	
	// Minimal damage applied to actors that are in the outer ring of radial damage
	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile|Damage")
	float MinimalDamage = 10.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile")
	float DamageInnerRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile")
	float DamageOuterRadius = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile|Impact|Rocket")
	TObjectPtr<USoundCue> RocketImpactSound;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile|Impact|Rocket")
	TObjectPtr<UParticleSystem> RocketImpactParticle;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> TrailComponent;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile|Sound")
	TObjectPtr<USoundCue> RocketSoundLoop;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile|Sound")
	TObjectPtr<USoundAttenuation> RocketAudioLoopAtt;
	
	UPROPERTY()
	TObjectPtr<UAudioComponent> RocketAudioLoopComp;

private:
	UPROPERTY(VisibleAnywhere, Category = "Blaster|Projectile")
	TObjectPtr<UStaticMeshComponent> RocketMesh;

	FTimerHandle DestroyTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile")
	float DestroyTime = 3.f;
};
