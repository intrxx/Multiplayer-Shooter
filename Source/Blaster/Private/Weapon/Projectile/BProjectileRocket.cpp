// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/BProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundCue.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraSystemInstanceController.h"

ABProjectileRocket::ABProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
	
	if(TrailSystem)
	{
		TrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailSystem, GetRootComponent(), FName(), GetActorLocation(),
			GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}

	if(RocketSoundLoop && RocketAudioLoopAtt)
	{
		RocketAudioLoopComp = UGameplayStatics::SpawnSoundAttached(RocketSoundLoop, GetRootComponent(), FName(),
			GetActorLocation(), EAttachLocation::KeepWorldPosition, false,
			0.6f, 1.f, 0.f, RocketAudioLoopAtt, nullptr,
			false);
	}
}

void ABProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}

void ABProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                               FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* FiringPawn = GetInstigator();
	if(FiringPawn && HasAuthority())
	{
		AController* FiringController = FiringPawn->GetController();
		if(FiringController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(this, Damage, MinimalDamage,
				GetActorLocation(), DamageInnerRadius, DamageOuterRadius, 1.f, UDamageType::StaticClass(),
				TArray<AActor*>(), this);
		}
	}
	
	GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &ThisClass::DestroyTimerFinished, DestroyTime);

	if(RocketImpactParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), RocketImpactParticle, GetActorTransform());
	}
	if(RocketImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), RocketImpactSound, GetActorLocation());
	}
	if(RocketMesh)
	{
		RocketMesh->SetVisibility(false);
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

