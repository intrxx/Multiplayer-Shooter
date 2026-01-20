// Copyright 2026 out of sCope team - intrxx


#include "Pickups/BPickupSpawner.h"
#include "Pickups/BPickup.h"

ABPickupSpawner::ABPickupSpawner()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void ABPickupSpawner::BeginPlay()
{
	Super::BeginPlay();

	SpawnPickup();
}

void ABPickupSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABPickupSpawner::SpawnPickup()
{
	int32 NumPickupClasses = PickupClasses.Num();
	
	if(NumPickupClasses > 0)
	{
		int32 RandSelection = FMath::RandRange(0, NumPickupClasses - 1);
		
		SpawnedPickup = GetWorld()->SpawnActor<ABPickup>(PickupClasses[RandSelection], GetActorTransform());

		if(HasAuthority() && SpawnedPickup)
		{
			SpawnedPickup->OnDestroyed.AddDynamic(this, &ThisClass::StartSpawnPickupTimer);
		}
	}
}

void ABPickupSpawner::SpawnPickupTimerFinished()
{
	if(HasAuthority())
	{
		SpawnPickup();
	}
}

void ABPickupSpawner::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	GetWorldTimerManager().SetTimer(SpawnPickupTimerHandle, this, &ThisClass::SpawnPickupTimerFinished,
		SpawnPickupTime);
}



