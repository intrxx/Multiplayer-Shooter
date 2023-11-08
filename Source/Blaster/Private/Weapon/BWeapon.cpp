// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/Projectile/BBulletShell.h"
#include "Net/UnrealNetwork.h"
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

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComp->SetupAttachment(RootComponent);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickUpWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpWidget"));
	PickUpWidgetComp->SetupAttachment(RootComponent);

	RandomRotationConstant = 30.f;
	FiringCrosshairErrorValue = 0.75f;
}

void ABWeapon::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
		SphereComp->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
	}

	if(PickUpWidgetComp)
	{
		PickUpWidgetComp->SetVisibility(false);
	}

	if(bCanChangeFiringMode && !FiringModes.IsEmpty())
	{
		FiringMode = FiringModes[0];
	}
}

void ABWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABWeapon, WeaponState);
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
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void ABWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
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
	
	switch(WeaponState)
	{
	case EBWeaponState::EWS_Equipped:
		ShowPickUpWidget(false);
		SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMeshComp->SetSimulatePhysics(false);
		WeaponMeshComp->SetEnableGravity(false);
		WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EBWeaponState::EWS_Dropped:
		if(HasAuthority())
		{
			SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMeshComp->SetSimulatePhysics(true);
		WeaponMeshComp->SetEnableGravity(true);
		WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	default:
		break;
	}
}

void ABWeapon::OnRep_WeaponState()
{
	switch(WeaponState)
	{
	case EBWeaponState::EWS_Equipped:
		ShowPickUpWidget(false);
		WeaponMeshComp->SetSimulatePhysics(false);
		WeaponMeshComp->SetEnableGravity(false);
		WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EBWeaponState::EWS_Dropped:
		WeaponMeshComp->SetSimulatePhysics(true);
		WeaponMeshComp->SetEnableGravity(true);
		WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	default:
		break;
	}
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
}

void ABWeapon::Dropped()
{
	SetWeaponState(EBWeaponState::EWS_Dropped);

	const FDetachmentTransformRules TransformRules(EDetachmentRule::KeepWorld, true);
	WeaponMeshComp->DetachFromComponent(TransformRules);

	SetOwner(nullptr);
}

