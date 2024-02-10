// Fill out your copyright notice in the Description page of Project Settings.

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

private:
	UPROPERTY(EditAnywhere, Category = "Blaster|Flag")
	TObjectPtr<UStaticMeshComponent> FlagMeshComp;
};
