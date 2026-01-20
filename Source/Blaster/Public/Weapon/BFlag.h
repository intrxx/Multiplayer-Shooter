// Copyright 2026 out of sCope team - intrxx

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BWeapon.h"
#include "BFlag.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABFlag : public ABWeapon
{
	GENERATED_BODY()

public:
	ABFlag();

	virtual void Dropped() override;

	void ResetFlag();

	FTransform GetInitialTransform() const {return InitialTransform;}

protected:
	virtual void HandleWeaponEquipped() override;
	virtual void HandleWeaponDropped() override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Blaster|Flag")
	TObjectPtr<UStaticMeshComponent> FlagMeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Blaster|Flag")
	FTransform InitialTransform;
};
