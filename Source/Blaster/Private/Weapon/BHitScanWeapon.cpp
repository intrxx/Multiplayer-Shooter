// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BHitScanWeapon.h"
#include "BlasterComponents/BLagCompensationComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Player/BPlayerController.h"
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

		FHitResult FireHit;
		HitScanTraceHit(Start, HitTarget, FireHit);
		
		if(ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor()))
		{
			if(InstigatorController)
			{
				if(HasAuthority() && (!bUseServerSideRewind || OwnerPawn->IsLocallyControlled()))
				{
					float CausedDamage = Damage;

					if(FireHit.BoneName.ToString() == FString("Head"))
					{
						CausedDamage = HeadShotDamage;
					}
					
					if(CheckLegsForHit(FireHit, BlasterCharacter->LegBoneNames))
					{
						CausedDamage = LegsShotDamage;
						//TODO Cause slow
					}
					
					UGameplayStatics::ApplyDamage(BlasterCharacter, CausedDamage, InstigatorController,
				this, UDamageType::StaticClass());
				}
				
				if(!HasAuthority() && bUseServerSideRewind)
				{
					BlasterCharacterOwner = BlasterCharacterOwner == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterCharacterOwner;
					BlasterControllerOwner = BlasterControllerOwner == nullptr ? Cast<ABPlayerController>(OwnerPawn->Controller) : BlasterControllerOwner;

					if(BlasterCharacterOwner && BlasterControllerOwner && BlasterCharacterOwner->GetLagCompensationComp() && BlasterCharacterOwner->IsLocallyControlled())
					{
						BlasterCharacterOwner->GetLagCompensationComp()->ServerHitScanScoreRequest(
							BlasterCharacter,
							Start,
							FireHit.ImpactPoint,
							BlasterControllerOwner->GetServerTimeSeconds() - BlasterControllerOwner->SingleTripTime);
					}
				}
			}
					
			if(CharacterImpactParticles && CharacterHitSound)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CharacterImpactParticles,
					FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), CharacterHitSound, FireHit.ImpactPoint);
			}
		}
		else
		{
			if(SurfaceImpactParticles && SurfaceHitSound)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SurfaceImpactParticles,
					FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), SurfaceHitSound, FireHit.ImpactPoint);
			}
		}

		if(MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}
		if(FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, GetActorLocation());
		}
	}
}

void ABHitScanWeapon::HitScanTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if(World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(OutHit, TraceStart, End, ECC_Visibility);

		FVector BeamEnd = End;
		if(OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			OutHit.ImpactPoint = End;
		}
		
		if(BeamParticles)
		{
			UParticleSystemComponent* BeamComp = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles,
				TraceStart, FRotator::ZeroRotator, true);
			if(BeamComp)
			{
				BeamComp->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}



