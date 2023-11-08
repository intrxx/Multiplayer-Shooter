// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BlasterTypes/BTurningInPlace.h"
#include "BlasterAnimInstance.generated.h"

class ABWeapon;
class ABlasterCharacter;
/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character", meta = (AllowPrivateAccess = true))
	ABlasterCharacter* BlasterCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character|Movement", meta = (AllowPrivateAccess = true))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character|Movement", meta = (AllowPrivateAccess = true))
	bool bIsInTheAir;

	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character|Movement", meta = (AllowPrivateAccess = true))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character|Movement", meta = (AllowPrivateAccess = true))
	bool bWeaponEquipped;

	TObjectPtr<ABWeapon> EquippedWeapon;
	
	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character|Movement", meta = (AllowPrivateAccess = true))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character|Movement", meta = (AllowPrivateAccess = true))
	bool bAiming;

	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character|Movement", meta = (AllowPrivateAccess = true))
	float YawOffset;
	
	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character|Movement", meta = (AllowPrivateAccess = true))
	float Lean;

	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character|Movement", meta = (AllowPrivateAccess = true))
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character|Movement", meta = (AllowPrivateAccess = true))
	float AO_Pitch;

	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character|Movement", meta = (AllowPrivateAccess = true))
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character|Movement", meta = (AllowPrivateAccess = true))
	EBTurningInPlace TurningInPlace;

	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character|Movement", meta = (AllowPrivateAccess = true))
	bool bLocallyControlled;
	
	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character|Movement", meta = (AllowPrivateAccess = true))
	FRotator RightHandRotation;
	
	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character|Movement", meta = (AllowPrivateAccess = true))
	bool bRotateRootBone;

	UPROPERTY(BlueprintReadOnly, Category = "Blaster|Character|Movement", meta = (AllowPrivateAccess = true))
	bool bHasDied;
	
	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;
	
};
