// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapon/BProjectile.h"
#include "BlasterGameplayStatics.h"

ABProjectileWeapon::ABProjectileWeapon()
{
}

void ABProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	
	const USkeletalMeshSocket* ProjSpawnSocket = GetWeaponMesh()->GetSocketByName(FName("ProjectileSpawnSocket"));
	if(ProjSpawnSocket)
	{
		FTransform SocketTransform = ProjSpawnSocket->GetSocketTransform(GetWeaponMesh());
		
		// From ProjSpawnSocket to Hit location from TraceUnderCrosshair
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();

		UBlasterGameplayStatics::BlasterDrawVectorDebugMessage(ToTarget, 5, FColor::Blue, FString("To Target"));
		
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
