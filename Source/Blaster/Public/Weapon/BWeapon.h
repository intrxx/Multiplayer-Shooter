// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlasterTypes/BWeaponTypes.h"
#include "BlasterTypes/BFiringMode.h"
#include "BWeapon.generated.h"

class USoundCue;
class ABBulletShell;
class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class UTexture2D;
class ABlasterCharacter;
class ABPlayerController;

UENUM(BlueprintType)
enum class EBWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equpped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equpped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "Default MAX")
};

UENUM(BlueprintType)
enum class EBFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_ProjectileWeapon UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),

	EFT_MAX UMETA(DisplayName = "Default MAX")
};

UCLASS()
class BLASTER_API ABWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	ABWeapon();
	
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;

	void SetHUDAmmo();
	void SetHUDAmmoImage();
	void ShowPickUpWidget(bool bShowWidget);
	void SetWeaponState(EBWeaponState State);
	
	virtual void Fire(const FVector& HitTarget);
	virtual void ChangeFiringMode();
	void Dropped();
	void AddAmmo(int32 AmmoToAdd);

	bool CheckLegsForHit(const FHitResult& HitResult, const TArray<FString>& BoneNames);
	
	/**
	 * Enable or disable custom depth to display the outline effect
	 */
	
	void EnableCustomDepth(bool bEnable);
	
	/**
	 * 
	 */

	FVector TraceEndWithScatter(const FVector& HitTarget);
	
	USphereComponent* GetWeaponSphereComp() const {return  SphereComp;}
	USkeletalMeshComponent* GetWeaponMesh() const {return  WeaponMeshComp;}
	float GetZoomedFOV() const {return ZoomedFOV;}
	float GetZoomInterpSpeed() const {return ZoomedInterpSpeed;}
	float GetShootingError() const {return FiringCrosshairErrorValue;}
	bool CanChangeFiringMode() const {return bCanChangeFiringMode;}
	EBFiringMode GetFiringMode() const {return FiringMode;}
	EBWeaponType GetWeaponType() const {return WeaponType;}
	bool IsMagEmpty() const;
	bool IsMagFull() const;
	int32 GetAmmo() const {return Ammo;}
	int32 GetMagCapacity() const {return MagCapacity;}
	float GetDamage() const {return Damage;}
	float GetHeadShotDamage() const {return HeadShotDamage;}
	float GetLegsShotDamage() const {return LegsShotDamage;}
	
public:
	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	EBFireType FireType;
	
	/**
	 * Textures for the weapon crosshair
	 */

	UPROPERTY(EditAnywhere, Category = "Blaster|Crosshair")
	TObjectPtr<UTexture2D> CrosshairDot;

	UPROPERTY(EditAnywhere, Category = "Blaster|Crosshair")
	TObjectPtr<UTexture2D> CrosshairLeft;

	UPROPERTY(EditAnywhere, Category = "Blaster|Crosshair")
	TObjectPtr<UTexture2D> CrosshairRight;

	UPROPERTY(EditAnywhere, Category = "Blaster|Crosshair")
	TObjectPtr<UTexture2D> CrosshairTop;

	UPROPERTY(EditAnywhere, Category = "Blaster|Crosshair")
	TObjectPtr<UTexture2D> CrosshairBottom;
	
	/**
	 *  Zoomed FOV while aiming
	 */

	UPROPERTY(EditAnywhere, Category = "Blaster|Aiming")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|Aiming")
	float ZoomedInterpSpeed = 20.f;
	
	/**
	 *
	 */

	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	float FireDelay = .15f;

	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	TObjectPtr<USoundCue> EmptyMagSound;

	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	TObjectPtr<USoundCue> EquipSound;

	bool bDestroyWeaponOnDeath = false;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon|Scatter")
	bool bUseScatter = false;

	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon|Image")
	TObjectPtr<UTexture2D> KillFeedGunImage;

protected:
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void HandleWeaponEquipped();
	virtual void HandleWeaponDropped();
	virtual void HandleSecondaryWeaponEquipped();

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);
	
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);
	
protected:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> BlasterCharacterOwner;
	UPROPERTY()
	TObjectPtr<ABPlayerController> BlasterControllerOwner;

	/**
	 * Trace End with Scatter
	 */

	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon|Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon|Scatter")
	float SphereRadius = 75.f;
	
	/**
	 * Client side prediction
	 */

	// The number of unprocessed server request for Ammo.
	// Incremented in SpendRound, decremented in UpdateAmmo
	int32 AmmoSequence = 0;

	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	float LegsShotDamage = 15.f;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	float Damage = 20.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	float HeadShotDamage = 40.f;
	
	UPROPERTY(EditAnywhere, Replicated, Category = "Blaster|Weapon")
	bool bUseServerSideRewind = false;

private:
	UFUNCTION()
	void OnRep_WeaponState();
	
	void SpendRound();
	
private:
	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	EBWeaponType WeaponType;
	
	UPROPERTY(VisibleAnywhere, Category = "Blaster|Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Blaster|Weapon")
	TObjectPtr<USphereComponent> SphereComp;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_WeaponState, Category = "Blaster|Weapon")
	EBWeaponState WeaponState;
	
	UPROPERTY(VisibleAnywhere, Category = "Blaster|Weapon")
	EBFiringMode FiringMode;

	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	TObjectPtr<UWidgetComponent> PickUpWidgetComp;

	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	TObjectPtr<UAnimationAsset> FireAnimation;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Weapon|FiringModes")
	bool bCanChangeFiringMode;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Weapon|FiringModes")
	TArray<EBFiringMode> FiringModes;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Weapon|FiringModes")
	TObjectPtr<USoundCue> ChangingModeSound;
	
	float FiringModeCount = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blaster|BulletShell")
	float FiringCrosshairErrorValue = 0.75f;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|BulletShell")
	TSubclassOf<ABBulletShell> BulletShell;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|BulletShell")
	float RandomRotationConstant = 30.f;
	
	/**
	 * Ammo
	 */

	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	int32 Ammo;

	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	int32 MagCapacity;
	
};

