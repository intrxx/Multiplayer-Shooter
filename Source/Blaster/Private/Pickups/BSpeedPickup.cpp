// Copyright 2026 out of sCope team - intrxx


#include "Pickups/BSpeedPickup.h"
#include "Character/BlasterCharacter.h"
#include "BlasterComponents/BlasterBuffComponent.h"

void ABSpeedPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if(BlasterCharacter)
	{
		UBlasterBuffComponent* BuffComp = BlasterCharacter->GetBuffComp();
		if(BuffComp)
		{
			BuffComp->SpeedBuff(BaseSpeedBuff, CrouchSpeedBuff, SpeedBuffTime);
		}
	}
	
	Destroy();
}
