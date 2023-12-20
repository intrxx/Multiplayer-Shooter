// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/BlasterBuffComponent.h"
#include "Character/BlasterCharacter.h"

UBlasterBuffComponent::UBlasterBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UBlasterBuffComponent::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void UBlasterBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealOverTime(DeltaTime);
}

void UBlasterBuffComponent::HealBuff(float HealAmount, float HealTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealTime;
	AmountToHeal = HealAmount;
}

void UBlasterBuffComponent::HealOverTime(float DeltaTime)
{
	if(!bHealing || BlasterCharacter && BlasterCharacter->IsDead())
	{
		return;
	}
	
	const float HealThisFrame = HealingRate * DeltaTime;
	
	AmountToHeal -= HealThisFrame;

	// I check first to make sure that Amount to Heal is greater then 0 to not overheal by one as the final if doesn't prevent that
	if(AmountToHeal > 0)
	{
		BlasterCharacter->SetHealth(FMath::Clamp(BlasterCharacter->GetHeath() + HealThisFrame, 0.f, BlasterCharacter->GetMaxHeath()));
		BlasterCharacter->UpdateHUDHealth();
	}
	
	if(AmountToHeal <= 0.f || BlasterCharacter->GetHeath() >= BlasterCharacter->GetMaxHeath())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

