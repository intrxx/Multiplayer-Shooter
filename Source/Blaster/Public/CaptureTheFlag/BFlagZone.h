// Copyright 2026 out of sCope team - intrxx

#pragma once

#include "CoreMinimal.h"
#include "BlasterTypes/BTeams.h"
#include "GameFramework/Actor.h"
#include "BFlagZone.generated.h"

class USphereComponent;

UCLASS()
class BLASTER_API ABFlagZone : public AActor
{
	GENERATED_BODY()
	
public:	
	ABFlagZone();

	UPROPERTY(EditAnywhere, Category = "Blaster|FlagZone")
	EBTeam ZoneTeam;
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent> ZoneSphereComp;
};
