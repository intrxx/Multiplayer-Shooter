// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BFlag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"

ABFlag::ABFlag()
{
	FlagMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMeshComp"));
	SetRootComponent(FlagMeshComp);

	GetWeaponSphereComp()->SetupAttachment(FlagMeshComp);
	GetPickupWidget()->SetupAttachment(FlagMeshComp);

	FlagMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
