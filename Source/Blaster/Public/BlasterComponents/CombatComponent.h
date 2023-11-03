// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class ABlasterHUD;
class ABPlayerController;
class ABlasterCharacter;
class ABWeapon;

namespace Combat
{
	static constexpr float TraceLength = 80000.0f;
	static constexpr float AimShrinkFactor = 0.55f;
}

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend ABlasterCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(ABWeapon* WeaponToEquip);
	
public:
	
protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonPressed(bool bPressed);

	// Server RPC, when called on client will execute on server
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	// When called on server it will run on all clients and server
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshair(FHitResult& OutHitResult, bool bUseDebug);

	void SetHUDCrosshair(float DeltaTime);

	void InterpFOV(float DeltaTime);

private:
	TObjectPtr<ABlasterCharacter> BlasterCharacter;
	TObjectPtr<ABPlayerController> BlasterPC;
	TObjectPtr<ABlasterHUD> BlasterHUD;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	TObjectPtr<ABWeapon> EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere, Category = "Blaster|Movement")
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere, Category = "Blaster|Movement")
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/**
	 *	HUD
	 */

	float CrosshairMovementFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

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
};
