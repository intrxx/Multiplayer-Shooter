// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Shotgun/BShotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void ABShotgun::Fire(const FVector& HitTarget)
{
	ABWeapon::Fire(HitTarget);
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if(OwnerPawn == nullptr)
	{
		return;
	}

	AController* InstigatorController = OwnerPawn->GetController();
	
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(TEXT("MuzzleFlash"));
	if(MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		
		TMap<ABlasterCharacter*, uint32> HitMap;
		
		for(uint32 i = 0; i < NumberOfPallets; i++)
		{
			FHitResult FireHit;
			HitScanTraceHit(Start, HitTarget, FireHit);
			
			if(ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor()))
			{
				if(HasAuthority() && InstigatorController)
				{	
					if(HitMap.Contains(BlasterCharacter))
					{
						HitMap[BlasterCharacter]++;
					}
					else
					{
						HitMap.Emplace(BlasterCharacter, 1);
					}
				}
				
				if(CharacterImpactParticles && CharacterHitSound)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CharacterImpactParticles,
						FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), CharacterHitSound, FireHit.ImpactPoint,
						0.5f, FMath::FRandRange(-0.5f, 0.5f));
				}
			}
			else
			{
				if(SurfaceImpactParticles && SurfaceHitSound)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SurfaceImpactParticles,
						FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), SurfaceHitSound, FireHit.ImpactPoint,
						0.5f, FMath::FRandRange(-0.5f, 0.5f));
				}
			}
		}
		for(auto HitPair : HitMap)
		{
			if(HitPair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value,
					InstigatorController, this, UDamageType::StaticClass());
			}
		}
	}
}
