// Copyright 2026 out of sCope team - intrxx


#include "Weapon/BFlag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Player/BPlayerController.h"
#include "Character/BlasterCharacter.h"

ABFlag::ABFlag()
{
	FlagMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMeshComp"));
	SetRootComponent(FlagMeshComp);

	GetWeaponSphereComp()->SetupAttachment(FlagMeshComp);
	GetPickupWidget()->SetupAttachment(FlagMeshComp);

	FlagMeshComp->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	FlagMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABFlag::BeginPlay()
{
	Super::BeginPlay();

	InitialTransform = GetActorTransform();
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

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetOwner());
	if(BlasterCharacter)
	{
		BlasterCharacter->SetHoldingTheFlag(false);
	}
	
	SetOwner(nullptr);
	BlasterCharacterOwner = nullptr;
	BlasterControllerOwner = nullptr;
}

void ABFlag::ResetFlag()
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetOwner());
	if(BlasterCharacter)
	{
		BlasterCharacter->SetHoldingTheFlag(false);
		BlasterCharacter->SetOverlappingWeapon(nullptr);
		BlasterCharacter->UnCrouch();
		BlasterCharacter->SwitchToFlagMappingContext(false);
	}

	if(!HasAuthority())
	{
		return;
	}
	
	SetActorTransform(InitialTransform);
	SetWeaponState(EBWeaponState::EWS_Initial);
	GetWeaponSphereComp()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetWeaponSphereComp()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	
	const FDetachmentTransformRules TransformRules(EDetachmentRule::KeepWorld, true);
	FlagMeshComp->DetachFromComponent(TransformRules);
	
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
	FlagMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagMeshComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
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

