// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BWeapon.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/BlasterCharacter.h"

ABWeapon::ABWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMeshComponent"));
	SetRootComponent(WeaponMeshComp);
	
	WeaponMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComp->SetupAttachment(RootComponent);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickUpWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpWidget"));
	PickUpWidgetComp->SetupAttachment(RootComponent);
	
}

void ABWeapon::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	}

	if(PickUpWidgetComp)
	{
		PickUpWidgetComp->SetVisibility(false);
	}
}

void ABWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter && PickUpWidgetComp)
	{
		PickUpWidgetComp->SetVisibility(true);
	}
}

void ABWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

