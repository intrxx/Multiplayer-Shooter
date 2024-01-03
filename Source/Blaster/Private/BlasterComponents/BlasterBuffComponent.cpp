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

void UBlasterBuffComponent::ShieldBuff(float ShieldAmount)
{
	if(BlasterCharacter == nullptr)
	{
		return;
	}

	if(!BlasterCharacter->IsDead())
	{
		BlasterCharacter->SetShield(FMath::Clamp(BlasterCharacter->GetShield() + ShieldAmount, 0.f, BlasterCharacter->GetMaxShield()));
		BlasterCharacter->UpdateHUDShield();
	}
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

void UBlasterBuffComponent::JumpBuff(float BuffJumpVelocity, float BuffTime)
{
	if(BlasterCharacter == nullptr)
	{
		return;
	}

	BlasterCharacter->GetWorldTimerManager().SetTimer(JumpBuffTimerHandle, this, &ThisClass::ResetJumpBuff,
		BuffTime);
	
	MulticastJumpBuff_Implementation(BuffJumpVelocity);
}

void UBlasterBuffComponent::ResetJumpBuff()
{
	MulticastJumpBuff_Implementation(InitialJumpVelocity);
}

void UBlasterBuffComponent::SetInitialSpeed(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBlasterBuffComponent::SetInitialJumpVelocity(float BaseJumpVelocity)
{
	InitialJumpVelocity = BaseJumpVelocity;
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

void UBlasterBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if(BlasterCharacter && BlasterCharacter->GetCharacterMovement())
	{
		BlasterCharacter->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}

void UBlasterBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if(BlasterCharacter && BlasterCharacter->GetCharacterMovement())
	{
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;

		if(BlasterCharacter->GetCombatComp())
		{
			BlasterCharacter->GetCombatComp()->SetBuffedSpeed(BaseSpeed);
		}
	}
}



