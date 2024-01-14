// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Shotgun/BShotgun.h"

#include "BlasterComponents/BLagCompensationComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"
#include "Player/BPlayerController.h"

void ABShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	ABWeapon::Fire(FVector());
	
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
		const FVector Start = SocketTransform.GetLocation();

		// Maps characters hit to number of times hit
		TMap<ABlasterCharacter*, uint32> HitMap;
		for(FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			HitScanTraceHit(Start, HitTarget, FireHit);
			
			if(ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor()))
			{
				if(HitMap.Contains(BlasterCharacter))
				{
					HitMap[BlasterCharacter]++;
				}
				else
				{
					HitMap.Emplace(BlasterCharacter, 1);
				}

				if(CharacterImpactParticles && CharacterHitSound)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CharacterImpactParticles,
						FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), CharacterHitSound, FireHit.ImpactPoint,
						0.5f, 0.9f);
				}
			}
			else
			{
				if(SurfaceImpactParticles && SurfaceHitSound)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SurfaceImpactParticles,
						FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), SurfaceHitSound, FireHit.ImpactPoint,
						0.5f, 0.9f);
				}
			}
		}
		TArray<ABlasterCharacter*> HitCharacters;
		
		for(auto HitPair : HitMap)
		{
			if(HitPair.Key && InstigatorController)
			{
				if(HasAuthority() && (!bUseServerSideRewind || OwnerPawn->IsLocallyControlled()))
				{
					UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value, // Multiply damage by the times hit
					InstigatorController, this, UDamageType::StaticClass());
				}
				HitCharacters.Add(HitPair.Key);
			}
		}
		
		if(!HasAuthority() && bUseServerSideRewind)
		{
			BlasterCharacterOwner = BlasterCharacterOwner == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterCharacterOwner;
			BlasterControllerOwner = BlasterControllerOwner == nullptr ? Cast<ABPlayerController>(OwnerPawn->Controller) : BlasterControllerOwner;
			if(BlasterCharacterOwner && BlasterControllerOwner && BlasterCharacterOwner->GetLagCompensationComp() && BlasterCharacterOwner->IsLocallyControlled())
			{
				BlasterCharacterOwner->GetLagCompensationComp()->ServerShotgunScoreRequest(HitCharacters, Start, HitTargets,
					BlasterControllerOwner->GetServerTimeSeconds() - BlasterControllerOwner->SingleTripTime);
			}
		}
		
	}
}

void ABShotgun::ShotgunScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& OutHitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(TEXT("MuzzleFlash"));
	if(!MuzzleFlashSocket)
	{
		return;
	}

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	for(uint32 i = 0; i < NumberOfPallets; i++)
	{
		const FVector RandVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLocation = SphereCenter + RandVector;
		const FVector ToEndLocation = EndLocation - TraceStart;
		
		OutHitTargets.Add(FVector(TraceStart + ToEndLocation * Combat::TraceLength / ToEndLocation.Size()));
	}
}


