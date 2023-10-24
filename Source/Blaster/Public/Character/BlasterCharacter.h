// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BlasterCharacter.generated.h"

class UCombatComponent;
class ABWeapon;
class UWidgetComponent;
class UBInputConfig;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	void SetOverlappingWeapon(ABWeapon* Weapon);

	FORCEINLINE float GetAO_Yaw() const {return AO_Yaw;}
	FORCEINLINE float GetAO_Pitch() const {return AO_Pitch;}
	ABWeapon* GetEquippedWeapon();
	bool IsWeaponEquipped();
	bool IsAiming();

protected:
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();

	void AimOffset(float DeltaTime);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blaster|Input")
	TObjectPtr<UBInputConfig> InputConfig;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blaster|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

private:
	UPROPERTY(VisibleAnywhere, Category = "Blaster|Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Blaster|Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Blaster|UI")
	TObjectPtr<UWidgetComponent> OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	TObjectPtr<ABWeapon> OverlappingWeapon;

	UPROPERTY(VisibleAnywhere, Category = "Blaster|Camera")
	TObjectPtr<UCombatComponent> CombatComp;

	float AO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

private:
	UFUNCTION()
	void OnRep_OverlappingWeapon(ABWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquip();
};
