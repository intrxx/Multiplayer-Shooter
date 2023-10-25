// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BProjectile.h"
#include "Components/BoxComponent.h"

ABProjectile::ABProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
}

void ABProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

