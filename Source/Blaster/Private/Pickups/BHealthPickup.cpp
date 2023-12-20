// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/BHealthPickup.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/BlasterCharacter.h"
#include "BlasterComponents/BlasterBuffComponent.h"

ABHealthPickup::ABHealthPickup()
{
	bReplicates = true;

	HealthPickupComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HealthPickupEffectComp"));
	HealthPickupComp->SetupAttachment(RootComponent);
}

void ABHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		UBlasterBuffComponent* BuffComp = BlasterCharacter->GetBuffComp();
		if(BuffComp)
		{
			BuffComp->HealBuff(HealAmount, HealingTime);
		}
	}
	
	Destroy();
}

void ABHealthPickup::Destroyed()
{
	if(HealthPickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, HealthPickupEffect, GetActorLocation(),
			GetActorRotation());
	}
	
	Super::Destroyed();
}
