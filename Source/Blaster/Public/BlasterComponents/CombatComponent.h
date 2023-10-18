// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class ABlasterCharacter;
class ABWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void EquipWeapon(ABWeapon* WeaponToEquip);
	
public:
	friend ABlasterCharacter;

private:
	TObjectPtr<ABlasterCharacter> BlasterCharacter;
	TObjectPtr<ABWeapon> EquippedWeapon;
	
protected:
	virtual void BeginPlay() override;

		
};
