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
	
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> BlasterCharacter;	
	
};
