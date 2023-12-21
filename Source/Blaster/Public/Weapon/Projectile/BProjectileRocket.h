// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile/BProjectile.h"
#include "BProjectileRocket.generated.h"

class USphereComponent;
class UBRocketProjectileMovementComp;
class UStaticMeshComponent;
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
	
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile|RocketJump")
	float RocketJumpImpulse = 800.f;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Projectile|RocketJump")
	float RocketJumpSelfDamage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile|Impact|Rocket")
	TObjectPtr<USoundCue> RocketImpactSound;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile|Impact|Rocket")
	TObjectPtr<UParticleSystem> RocketImpactParticle;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile|Sound")
	TObjectPtr<USoundCue> RocketSoundLoop;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Projectile|Sound")
	TObjectPtr<USoundAttenuation> RocketAudioLoopAtt;
	
	UPROPERTY()
	TObjectPtr<UAudioComponent> RocketAudioLoopComp;

private:
	UPROPERTY(VisibleAnywhere, Category = "Blaster|Projectile")
	TObjectPtr<UBRocketProjectileMovementComp> RocketProjectileMoveComp;
	
	// Used for the rocket jump logic
	UPROPERTY(VisibleAnywhere, Category = "Blaster|Projectile")
	TObjectPtr<USphereComponent> SphereComp;
};
