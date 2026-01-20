// Copyright 2026 out of sCope team - intrxx


#include "Pickups/BShieldPickup.h"

#include "BlasterComponents/BlasterBuffComponent.h"
#include "Character/BlasterCharacter.h"

void ABShieldPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		UBlasterBuffComponent* BuffComp = BlasterCharacter->GetBuffComp();
		if(BuffComp)
		{
			BuffComp->ShieldBuff(DefaultShieldReplenishAmount);
		}
	}
	
	Destroy();
}
