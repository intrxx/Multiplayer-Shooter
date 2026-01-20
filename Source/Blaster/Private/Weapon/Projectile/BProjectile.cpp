// Copyright 2026 out of sCope team - intrxx


#include "Weapon/Projectile/BProjectile.h"
#include "NiagaraFunctionLibrary.h"
#include "Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Physics/BlasterCollisionChannels.h"
#include "Sound/SoundCue.h"

ABProjectile::ABProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// This needs to be disabled in the Server Side Rewind Projectiles BPs
	bReplicates = true;
	
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_ObjectChannel_SkeletalMesh, ECR_Block);
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
}

void ABProjectile::SpawnTrailSystem()
{
	if(TrailSystem)
	{
		TrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailSystem, GetRootComponent(), FName(), GetActorLocation(),
			GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}
}

void ABProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &ThisClass::DestroyTimerFinished, DestroyTime);
}

void ABProjectile::DestroyTimerFinished()
{
	Destroy();
}

void ABProjectile::Destroyed()
{
	Super::Destroyed();
	
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, GetActorTransform());
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, GetActorLocation());
}

bool ABProjectile::ProjectileCheckLegsForHit(const FHitResult& HitResult, const TArray<FString>& BoneNames)
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

void ABProjectile::MulticastPlayHitParticleAndSound_Implementation(bool bCharacterHit)
{
	HitParticles = bCharacterHit ? CharacterImpactParticle : SurfaceImpactParticle;
	HitSound = bCharacterHit ? CharacterImpactSound : SurfaceImpactSound;

	Destroy();
}

void ABProjectile::ExplodeDamage()
{
	APawn* FiringPawn = GetInstigator();
	if(FiringPawn && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if(FiringController)
		{
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(GetOwner());
			
			UGameplayStatics::ApplyRadialDamageWithFalloff(this, Damage, MinimalDamage,
				GetActorLocation(), DamageInnerRadius, DamageOuterRadius, 1.f, UDamageType::StaticClass(),
				ActorsToIgnore, this);
		}
	}
}

