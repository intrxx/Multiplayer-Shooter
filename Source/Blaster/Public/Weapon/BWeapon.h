// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BWeapon.generated.h"

class ABBulletShell;
class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;

UENUM(BlueprintType)
enum class EBWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equpped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class BLASTER_API ABWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	ABWeapon();
	
	virtual void Tick(float DeltaTime) override;

	void ShowPickUpWidget(bool bShowWidget);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetWeaponState(EBWeaponState State);

	virtual void Fire(const FVector& HitTarget);
	
	FORCEINLINE USphereComponent* GetWeaponSphereComp() {return  SphereComp;}
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() {return  WeaponMeshComp;}
	
public:
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere, Category = "Blaster|Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Blaster|Weapon")
	TObjectPtr<USphereComponent> SphereComp;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_WeaponState, Category = "Blaster|Weapon")
	EBWeaponState WeaponState;

	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	TObjectPtr<UWidgetComponent> PickUpWidgetComp;

	UPROPERTY(EditAnywhere, Category = "Blaster|Weapon")
	TObjectPtr<UAnimationAsset> FireAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Weapon")
	TSubclassOf<ABBulletShell> BulletShell;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Weapon")
	float RandomRotationConstant;

private:
	UFUNCTION()
	void OnRep_WeaponState();
};

