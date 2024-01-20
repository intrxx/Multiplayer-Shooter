// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BlasterInterfaces/BCrosshairInteractionInterface.h"
#include "BlasterTypes/BCombatState.h"
#include "BlasterTypes/BTurningInPlace.h"
#include "BlasterTypes/BWeaponTypes.h"
#include "Components/TimelineComponent.h"
#include "BlasterCharacter.generated.h"

class UBoxComponent;
class UBLagCompensationComponent;
class UBlasterBuffComponent;
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

DECLARE_MULTICAST_DELEGATE(FOnLeftGame);

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
	virtual void Destroyed() override;

	void SetOverlappingWeapon(ABWeapon* Weapon);
	void PlayFireMontage(bool bAiming);
	void PlayDeathMontage(bool bAiming);
	void PlayReloadMontage();
	void PlaySwapMontage();
	void PlayThrowGrenadeMontage(const EBGrenadeCategory GrenadeCategory);
	void EquipDefaultWeapon();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowScopeWidget(bool bShowScope);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastHandleDeath(bool bPlayerLeftGame);

	// For just server functionality
	void HandleDeath(bool bPlayerLeftGame);
	
	float GetAO_Yaw() const {return AO_Yaw;}
	float GetAO_Pitch() const {return AO_Pitch;}
	EBTurningInPlace GetTurningInPlace() const {return TurningInPlace;}
	EBCombatState GetCombatState() const;
	ABWeapon* GetEquippedWeapon();
	UCameraComponent* GetFollowCamera() const {return CameraComponent;}
	UBCombatComponent* GetCombatComp() const {return CombatComp;}
	UBlasterBuffComponent* GetBuffComp() const {return BuffComponent;}
	UBLagCompensationComponent* GetLagCompensationComp() const {return LagCompensationComp;}
	FVector GetHitTarget() const;
	TArray<UInputMappingContext*> GetGameplayMappingContexts() const {return GameplayMappingContexts;};
	bool ShouldRotateRootBone() const {return bRotateRootBone;}
	bool IsDead() const {return bDead;}
	bool GetDisableGameplay() const {return bDisableGameplay;}
	bool IsLocallyReloading();
	UAnimMontage* GetReloadMontage() const {return ReloadWeaponMontage;}
	UStaticMeshComponent* GetAttachedGrenade() const {return AttachedGrenade;}
	
	/**
	 * Attributes Getters / Setters
	 */

	float GetHeath() const {return Health;}
	float GetMaxHeath() const {return MaxHealth;}

	float GetShield() const {return Shield;}
	float GetMaxShield() const {return MaxShield;}

	void SetHealth(float Amount) {Health = Amount;}
	void SetShield(float Amount) {Shield = Amount;}
	
	/**
	 *
	 */
	bool IsWeaponEquipped();
	bool IsAiming();
	
	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();

	/**
	 * Leaving the game
	 */

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

public:
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;
	
	UPROPERTY()
	TMap<FName, UBoxComponent*> HitCollisionBoxesMap;

	bool bFinishedSwapping = false;
	
	FOnLeftGame OnLeftGameDelegate;

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

	/**
	 * Hit Boxes used for server-side rewind
	 */

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> HeadBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> NeckBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> HipsBox;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> SpineBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> Spine1Box;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> Spine2Box;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> LeftArmBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> LeftForeArmBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> LeftHandBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> RightArmBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> RightForeArmBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> RightHandBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> LeftUpLegBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> LeftLegBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> LeftFootBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> RightUpLegBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> RightLegBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> RightFootBox;

	/**
	 * 
	 */

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
	void LethalGrenadeButtonPressed();
	void TacticalGrenadeButtonPressed();
	void SwapButtonPressed();
	
	void DropOrDestroyWeapon(ABWeapon* Weapon);
	
	void AimOffset(float DeltaTime);
	void SimProxiesTurn();
	void TurnInPlace(float DeltaTime);
	void CalculateAO_Pitch();
	float CalculateSpeed();
	
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

	/**
	 * Blaster Components
	 */
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Blaster|Combat")
	TObjectPtr<UBCombatComponent> CombatComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Blaster|Combat")
	TObjectPtr<UBlasterBuffComponent> BuffComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Blaster|Combat")
	TObjectPtr<UBLagCompensationComponent> LagCompensationComp;

	/**
	 * Anim Montages 
	 */
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Combat")
	TObjectPtr<UAnimMontage> FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Blaster|Combat")
	TObjectPtr<UAnimMontage> ReloadWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Blaster|Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditAnywhere, Category = "Blaster|Combat")
	TObjectPtr<UAnimMontage> ThrowGrenadeMontage;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Death")
	TObjectPtr<UAnimMontage> RareDeathMontage;

	// Rare Death Montage Chance - [0,100]
	UPROPERTY(EditAnywhere, Category = "Blaster|Death")
	int32 RareDeathChance = 20.f;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Death")
	TArray<TObjectPtr<UAnimMontage>> RegularDeathMontages_Hip;

	UPROPERTY(EditAnywhere, Category = "Blaster|Death")
	TArray<TObjectPtr<UAnimMontage>> RegularDeathMontages_Aim;

	UPROPERTY(EditAnywhere, Category = "Blaster|Combat")
	TObjectPtr<UAnimMontage> SwapWeaponMontage;
	
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
	 * Player attributes
	 */

	UPROPERTY(EditAnywhere, Category = "Blaster|Attributes")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Health, Category = "Blaster|Attributes")
	float Health = 100.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|Attributes")
	float MaxShield = 100.f;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Shield, Category = "Blaster|Attributes")
	float Shield = 50.f;
	
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
	 * Grenades
	 */

	UPROPERTY(VisibleAnywhere, Category = "Blaster|Combat")
	TObjectPtr<UStaticMeshComponent> AttachedGrenade;
	
	/**
	 * Default Weapon
	 */

	UPROPERTY(EditAnywhere, Category = "Blaster|Defaults")
	TSubclassOf<ABWeapon> DefaultWeaponClass;

	/**
	 * Leaving the game
	 */

	bool bLeftGame = false;
private:
	UFUNCTION()
	void OnRep_OverlappingWeapon(ABWeapon* LastWeapon);

	UFUNCTION()
	void OnRep_Health(float LastHealth);
	UFUNCTION()
	void OnRep_Shield(float LastShield);
	
	UFUNCTION(Server, Reliable)
	void ServerEquip();

	UFUNCTION(Server, Reliable)
	void ServerSwapButtonPressed();
	
	void HideCharacterIfCameraClose();
	void RespawnTimerFinished();

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();
};

