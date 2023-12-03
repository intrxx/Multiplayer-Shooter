// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BHitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "BlasterTypes/BWeaponTypes.h"
#include "Kismet/KismetMathLibrary.h"
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

		FHitResult FireHit;
		HitScanTraceHit(Start, HitTarget, FireHit);
		
		if(ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor()))
		{
			if(HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(BlasterCharacter, Damage, InstigatorController,
				this, UDamageType::StaticClass());
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
		FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;
		World->LineTraceSingleByChannel(OutHit, TraceStart, End, ECC_Visibility);

		FVector BeamEnd = End;
		if(OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
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

FVector ABHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	FVector RandVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	FVector EndLocation = SphereCenter + RandVector;
	FVector ToEndLocation = EndLocation - TraceStart;

	/*
	DrawDebugSphere(GetWorld(), EndLocation, 15.f, 12.f, FColor::Blue, true);
	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12.f, FColor::Red, true);
	DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLocation * Combat::TraceLength / ToEndLocation.Size()),
		FColor::Green, true);
	*/
	
	return FVector(TraceStart + ToEndLocation * Combat::TraceLength / ToEndLocation.Size());
}

