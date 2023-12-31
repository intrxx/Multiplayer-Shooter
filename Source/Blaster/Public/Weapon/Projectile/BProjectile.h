// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BProjectile.generated.h"

class USoundCue;
class UBoxComponent;
class UParticleSystem;
class UParticleSystemComponent;
class UNiagaraComponent;
class UNiagaraSystem;

UCLASS()
class BLASTER_API ABProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ABProjectile();
	
	virtual void Tick(float DeltaTime) override;
	// We can put the Hit logic here like playing sound and spawning particles, because we call Destroy() from OnHit
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	void SpawnTrailSystem();
	void StartDestroyTimer();
	void DestroyTimerFinished();
	void ExplodeDamage();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Blaster|Projectile")
	TObjectPtr<UStaticMeshComponent> ProjectileMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile")
	TObjectPtr<UBoxComponent> CollisionBox;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile|FX")
	TObjectPtr<UNiagaraSystem> TrailSystem;
	
	UPROPERTY()
	TObjectPtr<UParticleSystem> HitParticles;
	
	UPROPERTY()
	TObjectPtr<USoundCue> HitSound;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> TrailComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile|Impact")
	TObjectPtr<UParticleSystem> SurfaceImpactParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile|Impact")
	TObjectPtr<USoundCue> SurfaceImpactSound;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile|Damage")
	float Damage = 20.f;

	// Minimal damage applied to actors that are in the outer ring of radial damage
	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile|Explode")
	float MinimalDamage = 10.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile|Explode")
	float DamageInnerRadius = 75.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile|Explode")
	float DamageOuterRadius = 400.f;

	FTimerHandle DestroyTimerHandle;

private:
	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> ParticleSystemComp;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile")
	TObjectPtr<UParticleSystem> TracerParticle;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile|Impact")
	TObjectPtr<USoundCue> CharacterImpactSound;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile|Impact")
	TObjectPtr<UParticleSystem> CharacterImpactParticle;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile")
	float DestroyTime = 3.f;

private:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayHitParticleAndSound(bool bCharacterHit);
};
