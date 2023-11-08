// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BlasterInterfaces/BCrosshairInteractionInterface.h"
#include "BlasterTypes/BTurningInPlace.h"
#include "Components/TimelineComponent.h"
#include "BlasterCharacter.generated.h"

class ABPlayerController;
class UBCombatComponent;
class ABWeapon;
class UWidgetComponent;
class UBInputConfig;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UAnimMontage;
class AController;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IBCrosshairInteractionInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void OnRep_ReplicatedMovement() override;

	void SetOverlappingWeapon(ABWeapon* Weapon);
	void PlayFireMontage(bool bAiming);
	void PlayDeathMontage(bool bAiming);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastHandleDeath();

	// For just server functionality
	void HandleDeath();
	
	float GetAO_Yaw() const {return AO_Yaw;}
	float GetAO_Pitch() const {return AO_Pitch;}
	EBTurningInPlace GetTurningInPlace() const {return TurningInPlace;}
	ABWeapon* GetEquippedWeapon();
	UCameraComponent* GetFollowCamera() const {return CameraComponent;}
	FVector GetHitTarget() const;
	bool ShouldRotateRootBone() const {return bRotateRootBone;}
	bool IsDead() const {return bDead;}
	
	bool IsWeaponEquipped();
	bool IsAiming();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blaster|Input")
	TObjectPtr<UBInputConfig> InputConfig;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blaster|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

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
	void ChangeFiringModeButtonPressed();

	void AimOffset(float DeltaTime);
	void SimProxiesTurn();
	void TurnInPlace(float DeltaTime);
	void CalculateAO_Pitch();
	float CalculateSpeed();

	void UpdateHUDHealth();
	void PlayHitReactMontage();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
		AController* InstigatorController, AActor* DamageCauser);

	void CreateDeathDynamicMaterialInstances();

private:
	TObjectPtr<ABPlayerController> BlasterPC;
	
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
	TObjectPtr<UBCombatComponent> CombatComp;

	UPROPERTY(EditAnywhere, Category = "Blaster|Combat")
	TObjectPtr<UAnimMontage> FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Blaster|Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Combat|Death")
	TObjectPtr<UAnimMontage> RareDeathMontage;

	// Rare Death Montage Chance - [0,1]
	UPROPERTY(EditAnywhere, Category = "Blaster|Combat|Death")
	int32 RareDeathChance;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Combat|Death")
	TArray<TObjectPtr<UAnimMontage>> RegularDeathMontages_Hip;

	UPROPERTY(EditAnywhere, Category = "Blaster|Combat|Death")
	TArray<TObjectPtr<UAnimMontage>> RegularDeathMontages_Aim;
	
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	EBTurningInPlace TurningInPlace;
	bool bRotateRootBone;
	float TurnThreshold = 0.75f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementRep;

	/**
	 * Player health
	 */

	UPROPERTY(EditAnywhere, Category = "Blaster|Attributes")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Health, Category = "Blaster|Attributes")
	float Health = 100.f;
	
	bool bDead = false;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Combat|Death")
	float RespawnDelay;
	FTimerHandle RespawnTimerHandle;

	/**
	 *	Dissolve Effect
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTimelineComponent> DissolveTimelineComp;

	FOnTimelineFloat DissolveTrackDelegate;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Combat|Death")
	TObjectPtr<UCurveFloat> DissolveCurve;
	
	// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = "Blaster|Combat|Death")
	TObjectPtr<UMaterialInstanceDynamic> DissolveDynamicMaterialInstance_Body1;

	UPROPERTY(VisibleAnywhere, Category = "Blaster|Combat|Death")
	TObjectPtr<UMaterialInstanceDynamic> DissolveDynamicMaterialInstance_Body2;

	UPROPERTY(VisibleAnywhere, Category = "Blaster|Combat|Death")
	TObjectPtr<UMaterialInstanceDynamic> DissolveDynamicMaterialInstance_Head;

	// Material instance set on the Blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = "Blaster|Combat|Death")
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance_Body1;

	UPROPERTY(EditAnywhere, Category = "Blaster|Combat|Death")
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance_Body2;

	UPROPERTY(EditAnywhere, Category = "Blaster|Combat|Death")
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance_Head;

	/**
	 * 
	 */
	
private:
	UFUNCTION()
	void OnRep_OverlappingWeapon(ABWeapon* LastWeapon);

	UFUNCTION()
	void OnRep_Health();
	
	UFUNCTION(Server, Reliable)
	void ServerEquip();
	
	void HideCharacterIfCameraClose();
	void RespawnTimerFinished();

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();
};

