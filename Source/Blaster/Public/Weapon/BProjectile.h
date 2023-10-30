// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BProjectile.generated.h"

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

public:
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> CollisionBox;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBProjectileMovementComponent> ProjectileMoveComp;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UParticleSystem> Tracer;

	TObjectPtr<UParticleSystemComponent> ParticleSystemComp;
};
