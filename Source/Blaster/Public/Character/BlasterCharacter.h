// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BlasterInterfaces/CrosshairInteractionInterface.h"
#include "BlasterTypes/BTurningInPlace.h"
#include "BlasterCharacter.generated.h"

class UCombatComponent;
class ABWeapon;
class UWidgetComponent;
class UBInputConfig;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UAnimMontage;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public ICrosshairInteractionInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	void SetOverlappingWeapon(ABWeapon* Weapon);

	void PlayFireMontage(bool bAiming);
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHit();

	float GetAO_Yaw() const {return AO_Yaw;}
	float GetAO_Pitch() const {return AO_Pitch;}
	EBTurningInPlace GetTurningInPlace() const {return TurningInPlace;}
	ABWeapon* GetEquippedWeapon();
	UCameraComponent* GetFollowCamera() const {return CameraComponent;}
	FVector GetHitTarget() const;
	
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
	virtual void Jump() override;
	void FireWeaponPressed();
	void FireWeaponReleased();

	void AimOffset(float DeltaTime);
	void TurnInPlace(float DeltaTime);

	void PlayHitReactMontage();
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

	UPROPERTY(EditAnywhere, Category = "Blaster|Camera")
	float CameraThreshold = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Blaster|UI")
	TObjectPtr<UWidgetComponent> OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	TObjectPtr<ABWeapon> OverlappingWeapon;

	UPROPERTY(VisibleAnywhere, Category = "Blaster|Combat")
	TObjectPtr<UCombatComponent> CombatComp;

	UPROPERTY(EditAnywhere, Category = "Blaster|Combat")
	TObjectPtr<UAnimMontage> FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Blaster|Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;
	
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	EBTurningInPlace TurningInPlace;

private:
	UFUNCTION()
	void OnRep_OverlappingWeapon(ABWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquip();
	
	void HideCharacterIfCameraClose();
};

