// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HUD/BlasterHUD.h"
#include "BlasterTypes/BWeaponTypes.h"
#include "BlasterTypes/BCombatState.h"
#include "Components/ActorComponent.h"
#include "BCombatComponent.generated.h"

class ABGrenade;
class ABProjectile;
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

	void UpdateHUDGrenades(const EBGrenadeCategory GrenadeCategory);
	void UpdateHUDGrenadeImage(const EBGrenadeCategory GrenadeCategory);
	
	void EquipWeapon(ABWeapon* WeaponToEquip);
	void SwapWeapon();
	
	void Reload();
	UFUNCTION(BlueprintCallable, Category = "Blaster|Combat")
	void FinishReloading();
	
	UFUNCTION(BlueprintCallable, Category = "Blaster|Combat")
	void ShotgunShellReload();
	void JumpToShotGunMontageEnd();
	
	void FireButtonPressed(bool bPressed);

	UFUNCTION(BlueprintCallable, Category = "Blaster|Combat")
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable, Category = "Blaster|Combat")
	void LaunchGrenade();
	void ShowAttachedGrenade(bool bShow, UStaticMesh* GrenadeMesh = nullptr);
	
	void DropEquippedWeapon();

	void PickupAmmo(EBWeaponType WeaponType, int32 AmmoToAdd);

	int32 GetLethalGrenades() const {return CarriedLethalGrenades;}
	int32 GetTacticalGrenades() const {return CarriedTacticalGrenades;}
	ABGrenade* GetEquippedLethalGrenade() const {return EquippedLethalGrenade;}
	ABGrenade* GetEquippedTacticalGrenade() const {return EquippedTacticalGrenade;}

	void SetBuffedSpeed(float BaseSpeed) {BaseWalkSpeed = BaseSpeed;}

public:
	
protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	void AttachPrimaryWeapon(ABWeapon* WeaponToEquip);
	void AttachSecondaryWeapon(ABWeapon* WeaponToEquip);
	
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();

	UFUNCTION()
	void OnRep_EquippedLethalGrenade();

	UFUNCTION()
	void OnRep_EquippedTacticalGrenade();
	
	// Server RPC, when called on client will execute on server
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	void LocalFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerReload();
	void HandleReload();
	int32 CalculateAmountToReload();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade(const EBGrenadeCategory GrenadeCategory);
	void ThrowGrenade(const EBGrenadeCategory GrenadeCategory);
	
	// When called on server it will run on all clients and server
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshair(FHitResult& OutHitResult, bool bUseDebug);
	void SetHUDCrosshair(float DeltaTime);
	void InterpFOV(float DeltaTime);

	void AttachActorToHand(AActor* ActorToAttach, const FName SocketName);
	void UpdateCarriedAmmo();
	void PlayEquipWeaponSound(ABWeapon* Weapon);
	void ReloadEmptyWeapon();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	void AddDefaultGrenades();
	
private:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> BlasterCharacter;
	UPROPERTY()
	TObjectPtr<ABPlayerController> BlasterPC;
	UPROPERTY()
	TObjectPtr<ABlasterHUD> BlasterHUD;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	EBCombatState CombatState = EBCombatState::ECS_Unoccupied;

	UPROPERTY(Replicated)
	EBGrenadeCategory GrenadeTypeThrowing = EBGrenadeCategory::EGC_None;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	TObjectPtr<ABWeapon> EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	TObjectPtr<ABWeapon> SecondaryWeapon;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedLethalGrenade)
	TObjectPtr<ABGrenade> EquippedLethalGrenade;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedTacticalGrenade)
	TObjectPtr<ABGrenade> EquippedTacticalGrenade;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Defaults")
	TSubclassOf<ABGrenade> DefaultLethalGrenade;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Defaults")
	TSubclassOf<ABGrenade> DefaultTacticalGrenade;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere, Category = "Blaster|Movement")
	float BaseWalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|Movement")
	float AimWalkSpeedMultiplier = 0.8f;

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

	UPROPERTY(EditAnywhere, Category = "Blaster|Ammo")
	int32 StartingRifleAmmo = 30;

	UPROPERTY(EditAnywhere, Category = "Blaster|Ammo")
	int32 StartingRocketAmmo = 4;

	UPROPERTY(EditAnywhere, Category = "Blaster|Ammo")
	int32 StartingPistolAmmo = 20;

	UPROPERTY(EditAnywhere, Category = "Blaster|Ammo")
	int32 StartingSmgAmmo = 40;

	UPROPERTY(EditAnywhere, Category = "Blaster|Ammo")
	int32 StartingShotgunAmmo = 12;

	UPROPERTY(EditAnywhere, Category = "Blaster|Ammo")
	int32 StartingSniperAmmo = 10;

	UPROPERTY(EditAnywhere, Category = "Blaster|Ammo")
	int32 StartingGrenadeLauncherAmmo = 10;

	UPROPERTY(ReplicatedUsing = OnRep_CarriedTacticalGrenades)
	int32 CarriedTacticalGrenades = 3;
	
	UPROPERTY(ReplicatedUsing = OnRep_CarriedLethalGrenades)
	int32 CarriedLethalGrenades = 2;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Ammo|Grenades")
	int32 StartingFragGrenades = 1;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Ammo|Grenades")
	int32 StartingFlashGrenades = 2;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Ammo|Grenades")
	int32 StartingSemtexGrenades = 1;
	
	TMap<EBWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere, Category = "Blaster|Ammo")
	TMap<EBWeaponType, int32> MaxCarriedAmmoMap;
	
	TMap<EBGrenadeType, int32> CarriedGrenadesMap;
	

	/**
	 * Grenades
	 */

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Grenades")
	TSubclassOf<ABGrenade> FragGrenadeClass;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Grenades")
	TSubclassOf<ABGrenade> SemtexGrenadeClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Grenades")
	TSubclassOf<ABGrenade> FlashGrenadeClass;
	
private:
	void Fire();
	void StartFireTimer();
	void FireTimerFinished();
	void ShrinkCrosshairWhileShooting();
	void PlayScopeSounds(bool bIsAiming);

	bool CanFire();

	UFUNCTION()
	void OnRep_CarriedAmmo();
	void InitializeCarriedAmmo();

	UFUNCTION()
	void OnRep_CarriedLethalGrenades();
	UFUNCTION()
	void OnRep_CarriedTacticalGrenades();
	void InitializeCarriedGrenades();

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();
	void UpdateGrenadesValues(const EBGrenadeCategory GrenadeCategory);
};

