// Copyright 2026 out of sCope team - intrxx

#pragma once

#include "CoreMinimal.h"
#include "BlasterTypes/BWeaponTypes.h"
#include "Weapon/Projectile/BProjectileGrenade.h"
#include "BGrenade.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABGrenade : public ABProjectileGrenade
{
	GENERATED_BODY()

public:
	EBGrenadeType GetGrenadeType() const {return GrenadeType;}
	EBGrenadeCategory GetGrenadeCategory() const {return GrenadeCategory;}
	int32 GetMaxGrenades() const {return MaxGrenades;}
	UStaticMesh* GetGrenadeMesh() const {return GrenadeMesh;}

	UPROPERTY(EditAnywhere, Category = "Blaster|Grenades")
	TObjectPtr<UTexture2D> GrenadeHUDImage;
	
private:
	UPROPERTY(EditAnywhere, Category = "Blaster|Grenades")
	TObjectPtr<UStaticMesh> GrenadeMesh;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Grenade")
	EBGrenadeType GrenadeType;

	UPROPERTY(EditAnywhere, Category = "Blaster|Grenade")
	EBGrenadeCategory GrenadeCategory;

	UPROPERTY(EditAnywhere, Category = "Combat|Ammo|Grenades")
	int32 MaxGrenades = 3;
};
