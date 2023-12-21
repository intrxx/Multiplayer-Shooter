// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/BlasterBuffComponent.h"

#include "BlasterComponents/BCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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

void UBlasterBuffComponent::SpeedBuff(float BaseSpeedBuff, float CrouchSpeedBuff, float BuffTime)
{
	if(BlasterCharacter == nullptr)
	{
		return;
	}

	BlasterCharacter->GetWorldTimerManager().SetTimer(SpeedBuffTimerHandle, this, &ThisClass::ResetSpeedBuff,
		BuffTime);

	if(BlasterCharacter->GetCharacterMovement())
	{
		MulticastSpeedBuff(BaseSpeedBuff, CrouchSpeedBuff);
	}
}

void UBlasterBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBlasterBuffComponent::ResetSpeedBuff()
{
	if(BlasterCharacter && BlasterCharacter->GetCharacterMovement())
	{
		MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
	}
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

void UBlasterBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	BlasterCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;

	if(BlasterCharacter->GetCombatComp())
	{
		BlasterCharacter->GetCombatComp()->SetBuffedSpeed(BaseSpeed);
	}
}



