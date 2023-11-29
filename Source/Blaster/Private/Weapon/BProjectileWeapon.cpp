// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/Projectile/BProjectile.h"

void ABProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	// The weapon class has Authority only on the server because it is set to Replicated in the base class
	// If a class is not set to be replicated it has Authority on both server and client
	if(!HasAuthority())
	{
		return;
	}
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	
	const USkeletalMeshSocket* ProjSpawnSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if(ProjSpawnSocket)
	{
		FTransform SocketTransform = ProjSpawnSocket->GetSocketTransform(GetWeaponMesh());
		
		// From ProjSpawnSocket to Hit location from TraceUnderCrosshair
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		
		FRotator TargetRotation = ToTarget.Rotation();
		
		if(ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;
			
			UWorld* World = GetWorld();
			if(World)
			{
				World->SpawnActor<ABProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation,
					SpawnParams);
			}
		}
	}
}

