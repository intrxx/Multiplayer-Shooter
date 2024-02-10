// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BFlag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Player/BPlayerController.h"

ABFlag::ABFlag()
{
	FlagMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMeshComp"));
	SetRootComponent(FlagMeshComp);

	GetWeaponSphereComp()->SetupAttachment(FlagMeshComp);
	GetPickupWidget()->SetupAttachment(FlagMeshComp);

	FlagMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABFlag::Dropped()
{
	SetWeaponState(EBWeaponState::EWS_Dropped);

	const FDetachmentTransformRules TransformRules(EDetachmentRule::KeepWorld, true);
	FlagMeshComp->DetachFromComponent(TransformRules);

	if(BlasterControllerOwner)
	{
		BlasterControllerOwner->SetHUDWeaponTypeText(EBWeaponType::EWT_MAX);
	}
	
	SetOwner(nullptr);
	BlasterCharacterOwner = nullptr;
	BlasterControllerOwner = nullptr;
}

void ABFlag::HandleWeaponEquipped()
{
	ShowPickUpWidget(false);
	GetWeaponSphereComp()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FlagMeshComp->SetSimulatePhysics(false);
	FlagMeshComp->SetEnableGravity(false);
	FlagMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABFlag::HandleWeaponDropped()
{
	if(HasAuthority())
	{
		GetWeaponSphereComp()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	FlagMeshComp->SetSimulatePhysics(true);
	FlagMeshComp->SetEnableGravity(true);
	FlagMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FlagMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
	FlagMeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	FlagMeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}
