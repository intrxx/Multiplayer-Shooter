// Copyright 2026 out of sCope team - intrxx


#include "Pickups/BAmmoPickup.h"
#include "Character/BlasterCharacter.h"
#include "BlasterComponents/BCombatComponent.h"

void ABAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		UBCombatComponent* CombatComp = BlasterCharacter->GetCombatComp();
		if(CombatComp)
		{
			CombatComp->PickupAmmo(WeaponType, AmmoAmount);
		}
	}
	
	Destroy();
}
