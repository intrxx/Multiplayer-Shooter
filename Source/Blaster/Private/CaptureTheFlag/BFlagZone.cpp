// Copyright 2026 out of sCope team - intrxx


#include "CaptureTheFlag/BFlagZone.h"
#include "Components/SphereComponent.h"
#include "Game/BCaptureTheFlagGameMode.h"
#include "Weapon/BFlag.h"

ABFlagZone::ABFlagZone()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphereComp"));
	SetRootComponent(ZoneSphereComp);
}


void ABFlagZone::BeginPlay()
{
	Super::BeginPlay();

	ZoneSphereComp->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);
}

void ABFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABFlag* OverlappingFlag = Cast<ABFlag>(OtherActor);
	if(OverlappingFlag && OverlappingFlag->GetTeam() != ZoneTeam)
	{
		ABCaptureTheFlagGameMode* GameMode = GetWorld()->GetAuthGameMode<ABCaptureTheFlagGameMode>();
		if(GameMode)
		{
			GameMode->FlagCaptured(OverlappingFlag, this);
		}
		OverlappingFlag->ResetFlag();
	}
	
}


