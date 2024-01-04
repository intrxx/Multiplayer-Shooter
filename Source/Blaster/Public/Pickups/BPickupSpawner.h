// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BPickupSpawner.generated.h"

class ABPickup;

UCLASS()
class BLASTER_API ABPickupSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ABPickupSpawner();

	virtual void Tick(float DeltaTime) override;
	
public:
	
protected:
	virtual void BeginPlay() override;

	void SpawnPickup();
	void SpawnPickupTimerFinished();

	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);
	
protected:
	UPROPERTY(EditAnywhere, Category = "Blaster|PickupSpawner")
	TArray<TSubclassOf<ABPickup>> PickupClasses;

	UPROPERTY()
	TObjectPtr<ABPickup> SpawnedPickup;

private:
	UPROPERTY(EditAnywhere, Category = "Blaster|PickupSpawner")
	float SpawnPickupTime = 30.f;
	
	FTimerHandle SpawnPickupTimerHandle;
};
