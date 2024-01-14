// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/BWeapon.h"
#include "GameFramework/PlayerController.h"
#include "BPlayerController.generated.h"


class UBAnnouncement;
class ABlasterGameMode;
class UBInventoryWidget;
class UBScoreBoard;
class UBCharacterOverlay;
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

	UPROPERTY()            
	float Ping = 0.f;    
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHighPingDelegate, bool, bPingTooHigh);

/**
 * 
 */
UCLASS()
class BLASTER_API ABPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void ReceivedPlayer() override;

	/** Synced with server World clock */
	virtual float GetServerTimeSeconds();
	
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDInventoryCarriedAmmo(EBWeaponType WeaponType, int32 Ammo);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDWeaponAmmoImage(EBFiringMode FireMode);
	void SetHUDWeaponTypeText(EBWeaponType WeaponType);
	void SetHUDGameTimer(float CountdownTime);
	void SetHUDAnnouncementTimer(float CountdownTime);
	void SetDeathScreenVisibility(bool bSetVisibility);
	void SetHUDGrenadesNumber(int32 Grenades, const EBGrenadeCategory GrenadeCategory);
	void SetHUDGrenadesImage(UTexture2D* GrenadeImage, const EBGrenadeCategory GrenadeCategory);
	
	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();
	void HandleCooldown();
	
	//void SetHUDScore(float Score);
	UFUNCTION(Client, Reliable)
	void ClientSetHUDDeathScreen(const FString& KillerName);
	
	UFUNCTION(Client, Reliable)
	void ClientSetHUDPlayerStats(const TArray<FPlayerStats>& PlayerStats);

public:
	float SingleTripTime = 0.f;

	FOnHighPingDelegate OnHighPingDelegate;
	
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	void PollInit();
	
	void SetGameTime();

	void CheckPing(float DeltaSeconds);

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);
	
	void StartHighPingWarning();
	void StopHighPingWarning();

	/**
	 *	Sync time between client and server
	 */

	/** Requests the current server time passing in the client time when the request was sent */
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	/** Reports current server time to the client in response to ServerRequestServerTime */
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimerServerReceivedClientRequest);

	void CheckTimeSync(float DeltaTime);

	/**
	 *	
	 */

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float Warmup, float Match, float LevelStartingTime, float Cooldown);
	
protected:
	/** Difference between client and server time */
	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|Time")
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

private:
	UFUNCTION()
	void OnRep_MatchState();
	
private:
	UPROPERTY()
	TObjectPtr<ABlasterHUD> BlasterHUD;
	UPROPERTY()
	TObjectPtr<ABlasterGameMode> BlasterGameMode;

	float LevelStartedTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountDown = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UPROPERTY()
	UBCharacterOverlay* CharacterOverlay;
	UPROPERTY()
	UBScoreBoard* Scoreboard;
	UPROPERTY()
	UBInventoryWidget* InventoryWidget;
	UPROPERTY()
	UBAnnouncement* Announcement;
	
	TArray<FPlayerStats> LocalPlayerStats;

	UPROPERTY(EditAnywhere, Category = "Blaster|Ping")
	float HighPingThreshold = 75.f;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|Ping")
	float HighPingDuration = 5.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|Ping")
	float CheckPingFrequency = 20.f;
	
	float HighPingRunningTime = 0.f;
	float PingAnimationRunningTime = 0.f;
	
	/**
	 * For Poll Init
	*/

	bool bInitHealth = false;
	bool bInitShield = false;
	bool bInitGrenades = false;
	bool bInitScoreboard = false;
	bool bInitCarriedAmmo = false;
	bool bInitWeaponAmmo = false;
	bool bInitInventoryAmmo = false;
	bool bInitWeaponImage = false;
	
	float HUDHealth;
	float HUDMaxHealth;

	float HUDShield;
	float HUDMaxShield;

	float WeaponAmmo;
	float CarriedAmmo;
	float InventoryAmmo;
	EBWeaponType InventoryWeaponType;
	EBFiringMode FiringMode;

	int32 HUDLethalGrenades;
	int32 HUDTacticalGrenades;
	
};

