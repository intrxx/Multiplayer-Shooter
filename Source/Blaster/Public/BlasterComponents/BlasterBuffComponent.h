// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BlasterBuffComponent.generated.h"

class ABlasterCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBlasterBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBlasterBuffComponent();
	friend class ABlasterCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void HealBuff(float HealAmount, float HealTime = 0.f);
	
protected:
	virtual void BeginPlay() override;

	void HealOverTime(float DeltaTime);
private:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> BlasterCharacter;

	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;
	
};
