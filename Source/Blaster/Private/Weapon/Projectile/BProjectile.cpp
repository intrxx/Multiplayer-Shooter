// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/BProjectile.h"
#include "BlasterComponents/BProjectileMovementComponent.h"
#include "Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
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
	if(!bUseSameSoundAndParticles)
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
	}
	else
	{
		HitParticles = ImpactParticle;
		HitSound = ImpactSound;
		Destroy();
	}
}

void ABProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABProjectile::Destroyed()
{
	Super::Destroyed();
	
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, GetActorTransform());
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation());
}

void ABProjectile::MulticastPlayHitParticleAndSound_Implementation(bool bCharacterHit)
{
	HitParticles = bCharacterHit ? CharacterImpactParticle : SurfaceImpactParticle;
	HitSound = bCharacterHit ? CharacterImpactSound : SurfaceImpactSound;

	Destroy();
}

