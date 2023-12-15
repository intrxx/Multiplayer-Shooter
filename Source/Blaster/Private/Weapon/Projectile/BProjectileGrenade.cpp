// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/BProjectileGrenade.h"
#include "BlasterComponents/BProjectileMovementComponent.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ABProjectileGrenade::ABProjectileGrenade()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BProjectileMoveComp = CreateDefaultSubobject<UBProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	BProjectileMoveComp->SetIsReplicated(true);
	BProjectileMoveComp->bRotationFollowsVelocity = true;
	BProjectileMoveComp->bShouldBounce = true;
	
}

void ABProjectileGrenade::BeginPlay()
{
	AActor::BeginPlay();
	
	SpawnTrailSystem();
	StartDestroyTimer();

	if(BProjectileMoveComp)
	{
		BProjectileMoveComp->OnProjectileBounce.AddDynamic(this, &ThisClass::OnBounce);
	}

	HitParticles = SurfaceImpactParticle;
	HitSound = SurfaceImpactSound;
}

void ABProjectileGrenade::Destroyed()
{
	ExplodeDamage();
	
	Super::Destroyed();
}

void ABProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
	
	if(bShouldExplodeOnEnemy && Cast<ABlasterCharacter>(ImpactResult.GetActor()))
	{
		GetWorldTimerManager().ClearTimer(DestroyTimerHandle);
		Destroy();
	}
}
