// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BWeapon.h"
#include "GameFramework/PlayerController.h"
#include "BPlayerController.generated.h"


class ABlasterHUD;
class UTexture;

USTRUCT(BlueprintType)
struct FPlayerStats
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString PlayerName;
	
	UPROPERTY()
	float Score = 0.f;
	
	UPROPERTY()
	int32 Kills = 0.f;
	
	UPROPERTY()
	int32 Deaths = 0.f;
	
	UPROPERTY()
	int32 Assists = 0.f;
	
	UPROPERTY()
	float Damage = 0.f;
};

/**
 * 
 */
UCLASS()
class BLASTER_API ABPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDCarriedAmmo(int32 CarriedAmmo);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDWeaponAmmoImage(EBFiringMode FireMode);
	void SetHUDWeaponTypeText(EBWeaponType WeaponType);
	
	void SetDeathScreenVisibility(bool bSetVisibility);
	
	//void SetHUDScore(float Score);
	UFUNCTION(Client, Reliable)
	void ClientSetHUDDeathScreen(const FString& KillerName);
	
	UFUNCTION(Client, Reliable)
	void ClientSetHUDPlayerStats(const TArray<FPlayerStats>& PlayerStats);

	UFUNCTION()
	void OnRep_PlayerStats();
	
	void SetPlayerStats(TArray<FPlayerStats> Stats) {LocalPlayerStats = Stats;}

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerStats)
	TArray<FPlayerStats> LocalPlayerStats;

private:
	UPROPERTY()
	TObjectPtr<ABlasterHUD> BlasterHUD;
};

