// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BPickup.generated.h"

class USoundCue;
class USphereComponent;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS()
class BLASTER_API ABPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	ABPickup();
	
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;
	
public:
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, Category = "Blaster|FX")
	float BaseRotateSpeed = 45.f;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|FX")
	bool bShouldRotate = true;
	
private:
	UPROPERTY(EditAnywhere, Category = "Blaster|Pickup")
	TObjectPtr<USphereComponent> OverlapSphereComp;

	UPROPERTY(EditAnywhere, Category = "Blaster|Pickup|Sound")
	TObjectPtr<USoundCue> PickupSound;

	UPROPERTY(EditAnywhere, Category = "Blaster|Pickup")
	TObjectPtr<UStaticMeshComponent> PickupMeshComp;
	
	UPROPERTY(VisibleAnywhere, Category = "Blaster|Pickup|FX")
	TObjectPtr<UNiagaraComponent> HealthPickupComp;
    
	UPROPERTY(EditAnywhere, Category = "Blaster|Pickup|FX")
	TObjectPtr<UNiagaraSystem> HealthPickupEffect;
	
};
