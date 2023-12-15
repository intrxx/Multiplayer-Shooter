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
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "Default MAX")
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

	/**
	 * Enable or disable custom depth to display the outline effect
	 */
	
	void EnableCustomDepth(bool bEnable);
	
	/**
	 * 
	 */
	
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
	
public:
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

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> BlasterCharacterOwner;
	UPROPERTY()
	TObjectPtr<ABPlayerController> BlasterControllerOwner;

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

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo, Category = "Blaster|Weapon")
	int32 Ammo;

	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	int32 MagCapacity;

private:
	UFUNCTION()
	void OnRep_WeaponState();
	
	UFUNCTION()
	void OnRep_Ammo();
	void SpendRound();
};

