// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BWeapon.h"

#include "BlasterComponents/BCombatComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "BlasterTypes/BWeaponTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/Projectile/BBulletShell.h"
#include "Net/UnrealNetwork.h"
#include "Player/BPlayerController.h"
#include "Sound/SoundCue.h"

ABWeapon::ABWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	WeaponMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMeshComponent"));
	SetRootComponent(WeaponMeshComp);
	
	WeaponMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMeshComp->SetCustomDepthStencilValue(BlasterStencil::Purple);
	WeaponMeshComp->MarkRenderStateDirty();
	EnableCustomDepth(true);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComp->SetupAttachment(RootComponent);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SphereComp->SetSphereRadius(64.f);

	PickUpWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpWidget"));
	PickUpWidgetComp->SetupAttachment(RootComponent);
}

void ABWeapon::BeginPlay()
{
	Super::BeginPlay();

	SetReplicateMovement(true);
	
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	SphereComp->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
	
	if(PickUpWidgetComp)
	{
		PickUpWidgetComp->SetVisibility(false);
	}

	if(!FiringModes.IsEmpty())
	{
		FiringMode = FiringModes[0];
	}

	
}

void ABWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(ABWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

void ABWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	
	if(Owner == nullptr)
	{
		BlasterCharacterOwner == nullptr;
		BlasterControllerOwner == nullptr;
	}
	else
	{
		// Temporary fix because idk why Owners doesn't get nulled after we set the owner to null in BWeapon::Dropped()
		BlasterCharacterOwner = BlasterCharacterOwner == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterCharacterOwner;
		if(BlasterCharacterOwner && BlasterCharacterOwner->GetEquippedWeapon() && BlasterCharacterOwner->GetEquippedWeapon() == this)
		{
			BlasterControllerOwner = BlasterControllerOwner == nullptr ? Cast<ABPlayerController>(BlasterCharacterOwner->Controller) : BlasterControllerOwner;
			if(BlasterControllerOwner)
			{
				SetHUDAmmo();
				SetHUDAmmoImage();
			}
		}
	}
}

void ABWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		if(WeaponType == EBWeaponType::EWT_Flag && BlasterCharacter->GetTeam() != Team)
		{
			return;
		}
		if(BlasterCharacter->IsHoldingTheFlag())
		{
			return;
		}
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void ABWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		if(WeaponType == EBWeaponType::EWT_Flag && BlasterCharacter->GetTeam() != Team)
		{
			return;
		}
		if(BlasterCharacter->IsHoldingTheFlag())
		{
			return;
		}
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void ABWeapon::ShowPickUpWidget(bool bShowWidget)
{
	if(PickUpWidgetComp)
	{
		PickUpWidgetComp->SetVisibility(bShowWidget);
	}
}

void ABWeapon::SetWeaponState(EBWeaponState State)
{
	WeaponState = State;
	
	OnWeaponStateSet();
}

void ABWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}

void ABWeapon::OnWeaponStateSet()
{
	switch(WeaponState)
	{
	case EBWeaponState::EWS_Equipped:
		HandleWeaponEquipped();
		break;
	case EBWeaponState::EWS_EquippedSecondary:
		HandleSecondaryWeaponEquipped();
		break;
	case EBWeaponState::EWS_Dropped:
		HandleWeaponDropped();
		break;
	default:
		break;
	}
}

void ABWeapon::HandleWeaponEquipped()
{
	ShowPickUpWidget(false);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMeshComp->SetSimulatePhysics(false);
	WeaponMeshComp->SetEnableGravity(false);
	WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
	if(WeaponType == EBWeaponType::EWT_SubMachineGun)
	{
		WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMeshComp->SetEnableGravity(true);
		WeaponMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	EnableCustomDepth(false);

	BlasterCharacterOwner = BlasterCharacterOwner == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterCharacterOwner;
	if(BlasterCharacterOwner && bUseServerSideRewind && HasAuthority())
	{
		BlasterControllerOwner = BlasterControllerOwner == nullptr ? Cast<ABPlayerController>(BlasterCharacterOwner->Controller) : BlasterControllerOwner;
		if(BlasterControllerOwner && !BlasterControllerOwner->OnHighPingDelegate.IsBound())
		{
			BlasterControllerOwner->OnHighPingDelegate.AddDynamic(this, &ThisClass::OnPingTooHigh);
		}
	}
}

void ABWeapon::HandleWeaponDropped()
{
	if(HasAuthority())
	{
		SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMeshComp->SetSimulatePhysics(true);
	WeaponMeshComp->SetEnableGravity(true);
	WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMeshComp->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	WeaponMeshComp->SetCustomDepthStencilValue(BlasterStencil::Purple);
	WeaponMeshComp->MarkRenderStateDirty();
	EnableCustomDepth(true);

	BlasterCharacterOwner = BlasterCharacterOwner == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterCharacterOwner;
	
	if(BlasterCharacterOwner && HasAuthority())
	{
		BlasterControllerOwner = BlasterControllerOwner == nullptr ? Cast<ABPlayerController>(BlasterCharacterOwner->Controller) : BlasterControllerOwner;
		if(BlasterControllerOwner && BlasterControllerOwner->OnHighPingDelegate.IsBound())
		{
			BlasterControllerOwner->OnHighPingDelegate.RemoveDynamic(this, &ThisClass::OnPingTooHigh);
		}
	}
}

void ABWeapon::HandleSecondaryWeaponEquipped()
{
	ShowPickUpWidget(false);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMeshComp->SetSimulatePhysics(false);
	WeaponMeshComp->SetEnableGravity(false);
	WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
	if(WeaponType == EBWeaponType::EWT_SubMachineGun)
	{
		WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMeshComp->SetEnableGravity(true);
		WeaponMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	EnableCustomDepth(false);

	if(BlasterCharacterOwner && HasAuthority())
	{
		BlasterControllerOwner = BlasterControllerOwner == nullptr ? Cast<ABPlayerController>(BlasterCharacterOwner->Controller) : BlasterControllerOwner;
		if(BlasterControllerOwner && BlasterControllerOwner->OnHighPingDelegate.IsBound())
		{
			BlasterControllerOwner->OnHighPingDelegate.RemoveDynamic(this, &ThisClass::OnPingTooHigh);
		}
	}
}

void ABWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

void ABWeapon::Fire(const FVector& HitTarget)
{
	if(FireAnimation)
	{
		WeaponMeshComp->PlayAnimation(FireAnimation, false);
	}

	if(BulletShell)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMeshComp->GetSocketByName(FName("AmmoEject"));
		if(AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMeshComp);
			
			UWorld* World = GetWorld();
			if(World)
			{
				FRotator RandomRotation = SocketTransform.GetRotation().Rotator();
				
				RandomRotation = FRotator(
					FMath::RandRange(RandomRotation.Pitch - RandomRotationConstant, RandomRotation.Pitch + RandomRotationConstant),
					RandomRotation.Yaw,
					FMath::RandRange(RandomRotation.Roll - RandomRotationConstant, RandomRotation.Roll + RandomRotationConstant)
					);
				
				World->SpawnActor<ABBulletShell>(BulletShell, SocketTransform.GetLocation(), RandomRotation);
			}
		}
	}
	
	SpendRound();
}

void ABWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo-1, 0.f, MagCapacity);
	SetHUDAmmo();
	if(HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		AmmoSequence++;
	}
}

void ABWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

void ABWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if(HasAuthority())
	{
		return;
	}
	
	Ammo = ServerAmmo;
	AmmoSequence--;
	Ammo -= AmmoSequence;
	
	SetHUDAmmo();
}

void ABWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if(HasAuthority())
	{
		return;
	}
	
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	
	BlasterCharacterOwner = BlasterCharacterOwner == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterCharacterOwner;
	if(BlasterCharacterOwner && BlasterCharacterOwner->GetCombatComp() && IsMagFull())
	{
		BlasterCharacterOwner->GetCombatComp()->JumpToShotGunMontageEnd();
	}
	SetHUDAmmo();
}

void ABWeapon::ChangeFiringMode()
{
	if(FiringModes.IsEmpty())
	{
		return;
	}
	
	if(ChangingModeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ChangingModeSound, GetActorLocation());
	}
	
	if(FiringModeCount == FiringModes.Num()-1)
	{
		FiringModeCount = 0;
		FiringMode = FiringModes[FiringModeCount];
	}
	else
	{
		FiringModeCount++;
		FiringMode = FiringModes[FiringModeCount];
	}
	
	SetHUDAmmoImage();
}

void ABWeapon::SetHUDAmmo()
{
	BlasterCharacterOwner = BlasterCharacterOwner == nullptr ? Cast<ABlasterCharacter>(GetOwner())  : BlasterCharacterOwner;
	if(BlasterCharacterOwner)
	{
		BlasterControllerOwner = BlasterControllerOwner == nullptr ? Cast<ABPlayerController>(BlasterCharacterOwner->Controller) : BlasterControllerOwner;
		if(BlasterControllerOwner)
		{
			BlasterControllerOwner->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void ABWeapon::SetHUDAmmoImage()
{
	BlasterCharacterOwner = BlasterCharacterOwner == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterCharacterOwner;
	if(BlasterCharacterOwner)
	{
		BlasterControllerOwner = BlasterControllerOwner == nullptr ? Cast<ABPlayerController>(BlasterCharacterOwner->Controller) : BlasterControllerOwner;
		if(BlasterControllerOwner)
		{
			BlasterControllerOwner->SetHUDWeaponAmmoImage(FiringMode);
		}
	}	
}
void ABWeapon::Dropped()
{
	SetWeaponState(EBWeaponState::EWS_Dropped);

	const FDetachmentTransformRules TransformRules(EDetachmentRule::KeepWorld, true);
	WeaponMeshComp->DetachFromComponent(TransformRules);

	if(BlasterControllerOwner)
	{
		BlasterControllerOwner->SetHUDWeaponTypeText(EBWeaponType::EWT_MAX);
	}
	
	SetOwner(nullptr);
	BlasterCharacterOwner = nullptr;
	BlasterControllerOwner = nullptr;
}

void ABWeapon::EnableCustomDepth(bool bEnable)
{
	if(WeaponMeshComp)
	{
		WeaponMeshComp->SetRenderCustomDepth(bEnable);
	}
}

bool ABWeapon::IsMagEmpty() const
{
	return Ammo <= 0;
}

bool ABWeapon::IsMagFull() const
{
	return  Ammo == MagCapacity;
}

FVector ABWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(TEXT("MuzzleFlash"));
	if(!MuzzleFlashSocket)
	{
		return FVector(); 
	}

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLocation = SphereCenter + RandVector;
	const FVector ToEndLocation = EndLocation - TraceStart;

	/*
	DrawDebugSphere(GetWorld(), EndLocation, 15.f, 12.f, FColor::Blue, true);
	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12.f, FColor::Red, true);
	DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLocation * Combat::TraceLength / ToEndLocation.Size()),
		FColor::Green, true);
	*/
	
	return FVector(TraceStart + ToEndLocation * Combat::TraceLength / ToEndLocation.Size());
}

bool ABWeapon::CheckLegsForHit(const FHitResult& HitResult, const TArray<FString>& BoneNames)
{
	const FString BoneHit = HitResult.BoneName.ToString();
	
	for(auto& Name : BoneNames)
	{
		if(Name == BoneHit)
		{
			return true;
		}
	}
	return false;
}



