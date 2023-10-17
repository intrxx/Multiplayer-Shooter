// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BWeapon.h"

#include "Components/SphereComponent.h"

ABWeapon::ABWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMeshComponent"));
	WeaponMeshComp->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMeshComp);
	
	WeaponMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComp->SetupAttachment(RootComponent);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABWeapon::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
}

void ABWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

