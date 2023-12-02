// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BHitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

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
	if(MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25f;

		FHitResult FireHit;
		if(UWorld* World = GetWorld())
		{
			FVector BeamEnd = End;
			
			World->LineTraceSingleByChannel(FireHit, Start, End, ECC_Visibility);
			if(FireHit.bBlockingHit)
			{
				BeamEnd = FireHit.ImpactPoint;
				
				if(ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor()))
				{
					if(HasAuthority() && InstigatorController)
					{
						UGameplayStatics::ApplyDamage(BlasterCharacter, Damage, InstigatorController,
						this, UDamageType::StaticClass());
					}
					
					if(CharacterImpactParticles && CharacterHitSound)
					{
						UGameplayStatics::SpawnEmitterAtLocation(World, CharacterImpactParticles,
							FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
						UGameplayStatics::PlaySoundAtLocation(World, CharacterHitSound, FireHit.ImpactPoint);
					}
				}
				else
				{
					if(SurfaceImpactParticles && SurfaceHitSound)
					{
						UGameplayStatics::SpawnEmitterAtLocation(World, SurfaceImpactParticles,
							FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
						UGameplayStatics::PlaySoundAtLocation(World, SurfaceHitSound, FireHit.ImpactPoint);
					}
				}
			}

			if(BeamParticles)
			{
				UParticleSystemComponent* BeamComp = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles,
					SocketTransform);
				if(BeamComp)
				{
					BeamComp->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
			if(MuzzleFlash)
			{
				UGameplayStatics::SpawnEmitterAtLocation(World, MuzzleFlash, SocketTransform);
			}
			if(FireSound)
			{
				UGameplayStatics::PlaySoundAtLocation(World, FireSound, GetActorLocation());
			}
		}
	}
}
