// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/BProjectileBullet.h"
#include "Character/BlasterCharacter.h"
#include "Player/BPlayerController.h"
#include "BlasterComponents/BLagCompensationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BlasterComponents/BProjectileMovementComponent.h"
#include "Chaos/GeometryParticlesfwd.h"

ABProjectileBullet::ABProjectileBullet()
{
	BProjectileMoveComp = CreateDefaultSubobject<UBProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	BProjectileMoveComp->SetIsReplicated(true);
	BProjectileMoveComp->bRotationFollowsVelocity = true;
	BProjectileMoveComp->InitialSpeed = InitialProjectileSpeed;
	BProjectileMoveComp->MaxSpeed = MaxProjectileSpeed;
}

void ABProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	/*
	 * Example of well configured PredictProjectilePath, this is very important as Params have to be filled correctly 
	 *
	FPredictProjectilePathResult ProjPathResult;
	FPredictProjectilePathParams ProjPathParams;
	ProjPathParams.bTraceWithChannel = true;
	ProjPathParams.bTraceWithCollision = true;
	ProjPathParams.DrawDebugTime = 5.f;
	ProjPathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	ProjPathParams.LaunchVelocity = GetActorForwardVector() * InitialProjectileSpeed;
	ProjPathParams.MaxSimTime = 4.f;
	ProjPathParams.ProjectileRadius = 5.f;
	ProjPathParams.SimFrequency = 30.f;
	ProjPathParams.StartLocation = GetActorLocation();
	ProjPathParams.TraceChannel = ECC_Visibility;
	ProjPathParams.ActorsToIgnore.Add(this);
	
	UGameplayStatics::PredictProjectilePath(this, ProjPathParams, ProjPathResult);
	*/
}

#if WITH_EDITOR
void ABProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	
	if(PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, InitialProjectileSpeed))
	{
		if(BProjectileMoveComp)
		{
			BProjectileMoveComp->InitialSpeed = InitialProjectileSpeed;
		}
	}

	if(PropertyName == GET_MEMBER_NAME_CHECKED(ThisClass, MaxProjectileSpeed))
	{
		if(BProjectileMoveComp)
		{
			BProjectileMoveComp->MaxSpeed = MaxProjectileSpeed;
		}
	}
}
#endif

void ABProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                               FVector NormalImpulse, const FHitResult& Hit)
{
	ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
	if(OwnerCharacter)
	{
		ABPlayerController* OwnerController = Cast<ABPlayerController>(OwnerCharacter->Controller);
		if(OwnerController)
		{
			if(OwnerCharacter->HasAuthority() && (!bUseServerSideRewind || OwnerCharacter->IsLocallyControlled()))
			{
				UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this,
					UDamageType::StaticClass());
				
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}

			ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
			if(bUseServerSideRewind && OwnerCharacter->GetLagCompensationComp() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				OwnerCharacter->GetLagCompensationComp()->ServerProjectileScoreRequest(HitCharacter, TraceStart, InitialVelocity,
					OwnerController->GetServerTimeSeconds() - OwnerController->SingleTripTime);
			}
		}
	}
	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

