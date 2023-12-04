// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HUD/BlasterHUD.h"
#include "BlasterTypes/BWeaponTypes.h"
#include "BlasterTypes/BCombatState.h"
#include "Components/ActorComponent.h"
#include "BCombatComponent.generated.h"

class ABlasterHUD;
class ABPlayerController;
class ABlasterCharacter;
class ABWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBCombatComponent();
	friend ABlasterCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(ABWeapon* WeaponToEquip);
	void Reload();
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	void FireButtonPressed(bool bPressed);
	
public:
	
protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	
	// Server RPC, when called on client will execute on server
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerReload();
	void HandleReload();
	int32 CalculateAmountToReload();

	// When called on server it will run on all clients and server
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshair(FHitResult& OutHitResult, bool bUseDebug);
	void SetHUDCrosshair(float DeltaTime);
	void InterpFOV(float DeltaTime);

private:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> BlasterCharacter;
	UPROPERTY()
	TObjectPtr<ABPlayerController> BlasterPC;
	UPROPERTY()
	TObjectPtr<ABlasterHUD> BlasterHUD;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	EBCombatState CombatState = EBCombatState::ECS_Unoccupied;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	TObjectPtr<ABWeapon> EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere, Category = "Blaster|Movement")
	float BaseWalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|Movement")
	float AimWalkSpeed = 425.f;

	bool bFireButtonPressed;

	/**
	 *	HUD
	 */
	FCrosshairInfo CrosshairInfo;

	FVector HitTarget;
	
	float CrosshairMovementFactor = 0.f;
	float CrosshairInAirFactor = 0.f;
	float CrosshairAimFactor = 0.f;;
	float CrosshairShootingFactor = 0.f;;
	float CrosshairAimAtAnotherPlayerFactor = 0.f;;
	
	FLinearColor CrosshairColor;
	bool bAimingAtAnotherPlayer;
	
	/**
	 *	Aiming and FOV
	 */

	// FOV when not aiming set to camera's base FOV in BeginPlay
	float DefaultFOV;
	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Blaster|Aiming")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|Aiming")
	float UnZoomInterpSpeed = 20.f;

	/**
	 * Automatic Fire
	 */

	FTimerHandle FireTimer;
	bool bCanFire = true;

	// Carried ammo for the currently equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UPROPERTY(EditAnywhere, Category = "Combat|Ammo")
	int32 StartingRifleAmmo = 30;

	UPROPERTY(EditAnywhere, Category = "Combat|Ammo")
	int32 StartingRocketAmmo = 4;

	UPROPERTY(EditAnywhere, Category = "Combat|Ammo")
	int32 StartingPistolAmmo = 20;

	UPROPERTY(EditAnywhere, Category = "Combat|Ammo")
	int32 StartingSmgAmmo = 40;

	UPROPERTY(EditAnywhere, Category = "Combat|Ammo")
	int32 StartingShotgunAmmo = 12;

	UPROPERTY(EditAnywhere, Category = "Combat|Ammo")
	int32 StartingSniperAmmo = 10;

	TMap<EBWeaponType, int32> CarriedAmmoMap;

private:
	void Fire();
	void StartFireTimer();
	void FireTimerFinished();
	void ShrinkCrosshairWhileShooting();

	bool CanFire();

	UFUNCTION()
	void OnRep_CarriedAmmo();
	void InitializeCarriedAmmo();

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();
};
