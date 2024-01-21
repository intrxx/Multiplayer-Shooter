// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

class ABlasterGameState;
class ABlasterCharacter;
class ABPlayerState;
class ABPlayerController;
struct FPlayerStats;

namespace MatchState
{
	extern BLASTER_API const FName Cooldown; // Match technically ended, time for displaying information about the played match and begin cooldown timer
}
/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABlasterGameMode();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnPostLogin(AController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	void PlayerLeftGame(ABPlayerState* LeavingPlayer);
	
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABPlayerController* TargetBPC, ABPlayerController* AttackerBPC);
	virtual void RequestRespawn(ABlasterCharacter* CharacterToRespawn, AController* TargetBPC);

	float GetCountdownTime() const {return CountdownTime;}
	
	void UpdatePlayerList();
	void UpdateLeadingPlayer();

public:
	UPROPERTY()
	TArray<TObjectPtr<AController>> LoginPlayerControllers;

	UPROPERTY()
	TArray<FPlayerStats> PlayerStats;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Time")
	float MatchTime = 120.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Time")
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Time")
	float CooldownTime = 10.f;

	float LevelStartedTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
	
	void CalculateFurthestSpawnLocation(AActor*& OutSpawnPoint);
	
protected:
	UPROPERTY(EditAnywhere, Category = "Blaster|Gameplay")
	float KillScoreAward = 2.f;

private:
	float CountdownTime = 0.f;
};
