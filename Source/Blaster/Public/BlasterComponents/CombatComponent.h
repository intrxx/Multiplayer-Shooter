// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f;

class ABlasterCharacter;
class ABWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend ABlasterCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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
	void ServerFire();

	// When called on server it will run on all clients and server
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire();

	void TraceUnderCrosshair(FHitResult& OutHitResult);

private:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> BlasterCharacter;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	TObjectPtr<ABWeapon> EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere, Category = "Combat|Movement")
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere, Category = "Combat|Movement")
	float AimWalkSpeed;

	bool bFireButtonPressed;
};
