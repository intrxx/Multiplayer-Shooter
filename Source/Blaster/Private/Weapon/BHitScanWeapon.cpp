// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BHitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"

void ABHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr)
	{
		return;
	}

	AController* InstigatorController = OwnerPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(TEXT("MuzzleFlash"));
	if(MuzzleFlashSocket && InstigatorController)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25f;

		FHitResult FireHit;
		if(UWorld* World = GetWorld())
		{
			World->LineTraceSingleByChannel(FireHit, Start, End, ECC_Visibility);
			if(FireHit.bBlockingHit)
			{
				if(ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor()))
				{
					if(HasAuthority())
					{
						UGameplayStatics::ApplyDamage(BlasterCharacter, Damage, InstigatorController,
						this, UDamageType::StaticClass());
					}
					
					if(CharacterImpactParticles)
					{
						UGameplayStatics::SpawnEmitterAtLocation(World, CharacterImpactParticles,
							FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
					}
				}
				else
				{
					if(SurfaceImpactParticles)
					{
						UGameplayStatics::SpawnEmitterAtLocation(World, SurfaceImpactParticles,
							FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
					}
				}
			}
		}
	}
}
