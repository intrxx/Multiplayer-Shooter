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
		TMap<ABlasterCharacter*, uint32> BodyHitMap;
		TMap<ABlasterCharacter*, uint32> HeadHitMap;
		TMap<ABlasterCharacter*, uint32> LegsHitMap;
		
		for(FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			HitScanTraceHit(Start, HitTarget, FireHit);
			
			if(ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor()))
			{

				if(CheckLegsForHit(FireHit, BlasterCharacter->LegBoneNames))
				{
					if(LegsHitMap.Contains(BlasterCharacter))
					{
						LegsHitMap[BlasterCharacter]++;
					}
					else
					{
						LegsHitMap.Emplace(BlasterCharacter, 1);
					}
				}
				else if(FireHit.BoneName.ToString() == FString("Head"))
				{
					if(HeadHitMap.Contains(BlasterCharacter))
					{
						HeadHitMap[BlasterCharacter]++;
					}
					else
					{
						HeadHitMap.Emplace(BlasterCharacter, 1);
					}
				}
				else
				{
					if(BodyHitMap.Contains(BlasterCharacter))
					{
						BodyHitMap[BlasterCharacter]++;
					}
					else
					{
						BodyHitMap.Emplace(BlasterCharacter, 1);
					}
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

		// Maps Characters hit to total damage
		TMap<ABlasterCharacter*, float> DamageMap;

		// Calculate body shot damage
		for(auto BodyHitPair : BodyHitMap)
		{
			if(BodyHitPair.Key)
			{
				DamageMap.Emplace(BodyHitPair.Key, BodyHitPair.Value *  Damage);
				HitCharacters.AddUnique(BodyHitPair.Key);
			}
		}

		// Calculate head shot damage
		for(auto HeadHitPair : HeadHitMap)
		{
			if(HeadHitPair.Key)
			{
				if(DamageMap.Contains(HeadHitPair.Key))
				{
					DamageMap[HeadHitPair.Key] += HeadHitPair.Value * HeadShotDamage;
				}
				else
				{
					DamageMap.Emplace(HeadHitPair.Key, HeadHitPair.Value *  HeadShotDamage);
				}
				HitCharacters.AddUnique(HeadHitPair.Key);
			}
		}

		// Calculate leg shot damage
		for(auto LegHitPair : LegsHitMap)
		{
			if(LegHitPair.Key)
			{
				if(DamageMap.Contains(LegHitPair.Key))
				{
					DamageMap[LegHitPair.Key] += LegHitPair.Value * LegsShotDamage;
				}
				else
				{
					DamageMap.Emplace(LegHitPair.Key, LegHitPair.Value * LegsShotDamage);
				}
				HitCharacters.AddUnique(LegHitPair.Key);
			}
		}

		if(HasAuthority() && (!bUseServerSideRewind || OwnerPawn->IsLocallyControlled()))
		{
			for(auto DamagePair : DamageMap)
			{
				if(DamagePair.Key && InstigatorController)
				{
					UGameplayStatics::ApplyDamage(DamagePair.Key, DamagePair.Value, // Multiply damage by the times hit
					InstigatorController, this, UDamageType::StaticClass());
				}
			}
		}
		
		// Server Side Rewind
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


