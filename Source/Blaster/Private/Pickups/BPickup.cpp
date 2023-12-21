// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/BPickup.h"
#include "Sound/SoundCue.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"

ABPickup::ABPickup()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComp"));

	OverlapSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphereComp->SetupAttachment(RootComponent);
	OverlapSphereComp->SetSphereRadius(110.f);
	//OverlapSphereComp->AddLocalOffset(FVector(0.f, 0.f, 85.f));
	OverlapSphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	OverlapSphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	PickupMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMeshComp"));
	PickupMeshComp->SetupAttachment(OverlapSphereComp);
	PickupMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupNiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComp"));
	PickupNiagaraComp->SetupAttachment(RootComponent);
}

void ABPickup::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		OverlapSphereComp->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	}
}

void ABPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(PickupMeshComp && bShouldRotate)
	{
		PickupMeshComp->AddLocalRotation(FRotator(0.f, BaseRotateSpeed * DeltaTime, 0.f));
	}
}

void ABPickup::Destroyed()
{
	Super::Destroyed();

	if(PickupSound)
	{
		UGameplayStatics::PlaySound2D(this, PickupSound);
	}

	if(PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupEffect, GetActorLocation(),
			GetActorRotation());
	}
}

void ABPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}



