// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Projectile/BProjectileBullet.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "BlasterComponents/BProjectileMovementComponent.h"

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
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if(OwnerCharacter)
	{
		AController* OwnerController = OwnerCharacter->Controller;
		if(OwnerController)
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this,
				UDamageType::StaticClass());
		}
	}
	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

