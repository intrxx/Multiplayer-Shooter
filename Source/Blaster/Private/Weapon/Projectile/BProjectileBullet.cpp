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
	BProjectileMoveComp->InitialSpeed = 16000.f;
	BProjectileMoveComp->MaxSpeed = 16000.f;
}

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
