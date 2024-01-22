// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/Projectile/BProjectile.h"

void ABProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	
	const USkeletalMeshSocket* ProjSpawnSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	
	if(ProjSpawnSocket && World)
	{
		FTransform SocketTransform = ProjSpawnSocket->GetSocketTransform(GetWeaponMesh());
		
		// From ProjSpawnSocket to Hit location from TraceUnderCrosshair
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		
		FRotator TargetRotation = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;
		
		ABProjectile* SpawnedProjectile = nullptr;
		
		if(bUseServerSideRewind)
		{
			if(InstigatorPawn->HasAuthority()) // Server
			{
				if(InstigatorPawn->IsLocallyControlled()) // server, host - use replicated projectile, no need for ssr
				{
					SpawnedProjectile = World->SpawnActor<ABProjectile>(ProjectileClass, SocketTransform.GetLocation(),
						TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
					SpawnedProjectile->Damage = Damage;
					SpawnedProjectile->HeadShotDamage = HeadShotDamage;
					SpawnedProjectile->LegShotDamage = LegsShotDamage;
				}
				else // server, not locally controlled - spawn non-replicated projectile, ssr
				{
					SpawnedProjectile = World->SpawnActor<ABProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(),
						TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
				}
			}
			else // client, using ssr
			{
				if(InstigatorPawn->IsLocallyControlled()) // client, locally controlled - spawn non-replicated proj, use ssr
				{
					SpawnedProjectile = World->SpawnActor<ABProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(),
						TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialProjectileSpeed;
				}
				else // client, not locally controlled - spawn non-replicated projectile, no ssr
				{
					SpawnedProjectile = World->SpawnActor<ABProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(),
						TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		}
		else // weapon not using ssr
		{
			if(InstigatorPawn->HasAuthority())
			{
				SpawnedProjectile = World->SpawnActor<ABProjectile>(ProjectileClass, SocketTransform.GetLocation(),
						TargetRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->Damage = Damage;
				SpawnedProjectile->HeadShotDamage = HeadShotDamage;
				SpawnedProjectile->LegShotDamage = LegsShotDamage;
			}
		}
	}
}

