// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BProjectile.generated.h"

class USoundCue;
class UBoxComponent;
class UBProjectileMovementComponent;
class UParticleSystem;
class UParticleSystemComponent;

UCLASS()
class BLASTER_API ABProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	ABProjectile();
	
	virtual void Tick(float DeltaTime) override;
	// We can put the Hit logic here like playing sound and spawning particles, because we call Destroy() from OnHit
	virtual void Destroyed() override;

public:
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

protected:
	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile|Damage")
	float Damage;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile")
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBProjectileMovementComponent> ProjectileMoveComp;

	UPROPERTY()
	TObjectPtr<UParticleSystemComponent> ParticleSystemComp;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile")
	TObjectPtr<UParticleSystem> TracerParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile|Impact")
	TObjectPtr<UParticleSystem> MetalImpactParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile|Impact")
	TObjectPtr<UParticleSystem> CharacterImpactParticle;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile|Impact")
	TObjectPtr<USoundCue> SurfaceImpactSound;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile|Impact")
	TObjectPtr<USoundCue> CharacterImpactSound;

	UPROPERTY()
	TObjectPtr<UParticleSystem> HitParticles;
	
	UPROPERTY()
	TObjectPtr<USoundCue> HitSound;

private:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayHitParticleAndSound(bool bCharacterHit);
};
