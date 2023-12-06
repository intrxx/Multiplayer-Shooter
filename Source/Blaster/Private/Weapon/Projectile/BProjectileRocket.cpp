// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/BProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraSystemInstanceController.h"
#include "BlasterComponents/BRocketProjectileMovementComp.h"
#include "Character/BlasterCharacter.h"
#include "Components/SphereComponent.h"

ABProjectileRocket::ABProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketProjectileMoveComp = CreateDefaultSubobject<UBRocketProjectileMovementComp>(TEXT("RocketMoveComp"));
	RocketProjectileMoveComp->SetIsReplicated(true);
	RocketProjectileMoveComp->bRotationFollowsVelocity = true;
	RocketProjectileMoveComp->InitialSpeed = 7000.f;
	RocketProjectileMoveComp->MaxSpeed = 7000.f;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("RocketJumpSphereComp"));
	SphereComp->SetupAttachment(RootComponent);
	SphereComp->SetSphereRadius(0.f, false);
	
}

void ABProjectileRocket::Destroyed()
{
}

void ABProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	if(!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	}

	if(HasAuthority())
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
	}

	SpawnTrailSystem();
	
	if(RocketSoundLoop && RocketAudioLoopAtt)
	{
		RocketAudioLoopComp = UGameplayStatics::SpawnSoundAttached(RocketSoundLoop, GetRootComponent(), FName(),
			GetActorLocation(), EAttachLocation::KeepWorldPosition, false,
			0.6f, 1.f, 0.f, RocketAudioLoopAtt, nullptr,
			false);
	}
}

void ABProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                               FVector NormalImpulse, const FHitResult& Hit)
{
	if(OtherActor == GetOwner())
	{
		return;
	}

	SphereComp->SetSphereRadius(400.f, true);
		
	ExplodeDamage();
	StartDestroyTimer();
	
	if(RocketImpactParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), RocketImpactParticle, GetActorTransform());
	}
	if(RocketImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), RocketImpactSound, GetActorLocation());
	}
	if(ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	if(CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if(TrailComponent && TrailComponent->GetSystemInstanceController())
	{
		TrailComponent->GetSystemInstanceController()->Deactivate();
	}
	if(RocketAudioLoopComp && RocketAudioLoopComp->IsPlaying())
	{
		RocketAudioLoopComp->Stop();
	}
}

void ABProjectileRocket::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter && BlasterCharacter == GetOwner())
	{
		UGameplayStatics::ApplyDamage(BlasterCharacter, RocketJumpSelfDamage, BlasterCharacter->Controller,
			BlasterCharacter, UDamageType::StaticClass());
		
		FVector FromLocation = GetActorLocation();
		FVector PlayerLocation = BlasterCharacter->GetActorLocation();

		FVector LaunchVector = PlayerLocation - FromLocation;
		LaunchVector = LaunchVector.GetSafeNormal();
		LaunchVector *= RocketJumpImpulse;
		BlasterCharacter->LaunchCharacter(LaunchVector, true, false);
	}
}

