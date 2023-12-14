// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BlasterInterfaces/BCrosshairInteractionInterface.h"
#include "BlasterTypes/BCombatState.h"
#include "BlasterTypes/BTurningInPlace.h"
#include "Components/TimelineComponent.h"
#include "BlasterCharacter.generated.h"

class ABPlayerState;
class ABlasterHUD;
class USoundCue;
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
	void PlayReloadMontage();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowScopeWidget(bool bShowScope);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastHandleDeath();

	// For just server functionality
	void HandleDeath();
	
	float GetAO_Yaw() const {return AO_Yaw;}
	float GetAO_Pitch() const {return AO_Pitch;}
	EBTurningInPlace GetTurningInPlace() const {return TurningInPlace;}
	EBCombatState GetCombatState() const;
	ABWeapon* GetEquippedWeapon();
	UCameraComponent* GetFollowCamera() const {return CameraComponent;}
	UBCombatComponent* GetCombatComp() const {return CombatComp;}
	FVector GetHitTarget() const;
	TArray<UInputMappingContext*> GetGameplayMappingContexts() const {return GameplayMappingContexts;};
	bool ShouldRotateRootBone() const {return bRotateRootBone;}
	bool IsDead() const {return bDead;}
	bool GetDisableGameplay() const {return bDisableGameplay;}
	UAnimMontage* GetReloadMontage() const {return ReloadWeaponMontage;}
	
	/**
	 * Attributes Getters
	 */

	float GetHeath() const {return Health;}
	float GetMaxHeath() const {return MaxHealth;}
	
	/**
	 **
	 */
	bool IsWeaponEquipped();
	bool IsAiming();

	virtual void Destroyed() override;

public:
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

protected:
	TArray<UInputMappingContext*> GameplayMappingContexts;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blaster|Input")
	TObjectPtr<UBInputConfig> InputConfig;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blaster|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blaster|Input")
	TObjectPtr<UInputMappingContext> InventoryMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Blaster|Input")
	TObjectPtr<UInputMappingContext> CooldownStateMappingContext;

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
	void ReloadButtonPressed();
	void ChangeFiringModeButtonPressed();
	void ToggleScoreBoard();
	void ToggleInventory();
	
	void AimOffset(float DeltaTime);
	void SimProxiesTurn();
	void TurnInPlace(float DeltaTime);
	void CalculateAO_Pitch();
	float CalculateSpeed();

	void UpdateHUDHealth();
	void PlayHitReactMontage();

	void RotateInPlace(float DeltaTime);
	

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
		AController* InstigatorController, AActor* DamageCauser);

	void CreateDeathDynamicMaterialInstances();

	// Poll for any relevant classes and initialize HUD
	void PollInit();

private:
	UPROPERTY()
	TObjectPtr<ABPlayerController> BlasterPC;
	UPROPERTY()
	TObjectPtr<ABlasterHUD> BlasterHUD;
	UPROPERTY()
	TObjectPtr<ABPlayerState> BlasterPS;
	
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess), Category = "Blaster|Combat")
	TObjectPtr<UBCombatComponent> CombatComp;

	/**
	 * Anim Montages 
	 */
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Combat")
	TObjectPtr<UAnimMontage> FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Blaster|Combat")
	TObjectPtr<UAnimMontage> ReloadWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Blaster|Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Death")
	TObjectPtr<UAnimMontage> RareDeathMontage;

	// Rare Death Montage Chance - [0,1]
	UPROPERTY(EditAnywhere, Category = "Blaster|Death")
	int32 RareDeathChance = 20.f;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Death")
	TArray<TObjectPtr<UAnimMontage>> RegularDeathMontages_Hip;

	UPROPERTY(EditAnywhere, Category = "Blaster|Death")
	TArray<TObjectPtr<UAnimMontage>> RegularDeathMontages_Aim;

	/**
	 * 
	 */
	
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

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Death")
	float RespawnDelay = 3.f;
	FTimerHandle RespawnTimerHandle;

	/**
	 *	Dissolve Effect
	 */
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTimelineComponent> DissolveTimelineComp;

	FOnTimelineFloat DissolveTrackDelegate;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Death|Dissolve")
	TObjectPtr<UCurveFloat> DissolveCurve;
	
	// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = "Blaster|Death|Dissolve")
	TObjectPtr<UMaterialInstanceDynamic> DissolveDynamicMaterialInstance_Body1;

	UPROPERTY(VisibleAnywhere, Category = "Blaster|Death|Dissolve")
	TObjectPtr<UMaterialInstanceDynamic> DissolveDynamicMaterialInstance_Body2;

	UPROPERTY(VisibleAnywhere, Category = "Blaster|Death|Dissolve")
	TObjectPtr<UMaterialInstanceDynamic> DissolveDynamicMaterialInstance_Head;

	// Material instance set on the Blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = "Blaster|Death|Dissolve")
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance_Body1;

	UPROPERTY(EditAnywhere, Category = "Blaster|Death|Dissolve")
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance_Body2;

	UPROPERTY(EditAnywhere, Category = "Blaster|Death|Dissolve")
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance_Head;

	/**
	 * Death Bot
	 */

	UPROPERTY(EditAnywhere, Category = "Blaster|Death|DeathBot")
	TObjectPtr<UParticleSystem> DeathBotEffect;

	UPROPERTY(EditAnywhere, Category = "Blaster|Death|DeathBot")
	TObjectPtr<UParticleSystemComponent> DeathBotEffectComp;

	UPROPERTY(EditAnywhere, Category = "Blaster|Death|DeathBot")
	TObjectPtr<USoundCue> DeathBotSound;

	UPROPERTY(Replicated)
	float DeathBotSpawnZOffset = 115.f;
	
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

