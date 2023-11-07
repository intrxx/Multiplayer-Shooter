// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/BProjectile.h"
#include "BlasterComponents/BProjectileMovementComponent.h"
#include "Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Physics/BlasterCollisionChannels.h"
#include "Sound/SoundCue.h"

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
	CollisionBox->SetCollisionResponseToChannel(ECC_ObjectChannel_SkeletalMesh, ECR_Block);
	
	ProjectileMoveComp = CreateDefaultSubobject<UBProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMoveComp->bRotationFollowsVelocity = true;
	ProjectileMoveComp->InitialSpeed = 16000.f;
	ProjectileMoveComp->MaxSpeed = 16000.f;
	
	Damage = 20.f;
}

void ABProjectile::BeginPlay()
{
	Super::BeginPlay();

	if(TracerParticle)
	{
		ParticleSystemComp = UGameplayStatics::SpawnEmitterAttached(TracerParticle, CollisionBox, NAME_None,
			GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition);
	}

	if(HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	}
}

void ABProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		MulticastPlayHitParticleAndSound(true);
	}
	else
	{
		MulticastPlayHitParticleAndSound(false);
	}
	
	Destroy();
}

void ABProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABProjectile::Destroyed()
{
	Super::Destroyed();
}

void ABProjectile::MulticastPlayHitParticleAndSound_Implementation(bool bCharacterHit)
{
	if(bCharacterHit)
	{
		if(CharacterImpactParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CharacterImpactParticle, GetActorTransform());
		}

		if(CharacterImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), CharacterImpactSound, GetActorLocation());
		}
	}
	else
	{
		if(MetalImpactParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MetalImpactParticle, GetActorTransform());
		}

		if(SurfaceImpactSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), SurfaceImpactSound, GetActorLocation());
		}
	}
}

