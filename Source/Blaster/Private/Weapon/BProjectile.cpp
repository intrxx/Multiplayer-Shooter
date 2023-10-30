// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BProjectile.h"
#include "Particles/ParticleSystemComponent.h"
#include "BlasterComponents/BProjectileMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

ABProjectile::ABProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	
	ProjectileMoveComp = CreateDefaultSubobject<UBProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMoveComp->bRotationFollowsVelocity = true;
	ProjectileMoveComp->InitialSpeed = 16000.f;
	ProjectileMoveComp->MaxSpeed = 16000.f;
}

void ABProjectile::BeginPlay()
{
	Super::BeginPlay();

	if(Tracer)
	{
		ParticleSystemComp = UGameplayStatics::SpawnEmitterAttached(Tracer, CollisionBox, NAME_None,
			GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition);
	}
}

void ABProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

