// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

class ABlasterGameState;
class ABlasterCharacter;
class ABPlayerController;
struct FPlayerStats;

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
	
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABPlayerController* TargetBPC, ABPlayerController* AttackerBPC);
	virtual void RequestRespawn(ABlasterCharacter* CharacterToRespawn, AController* TargetBPC);

	virtual void OnPostLogin(AController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	
	void UpdatePlayerList();

public:
	UPROPERTY()
	TArray<TObjectPtr<AController>> LoginPlayerControllers;

	UPROPERTY()
	TArray<FPlayerStats> PlayerStats;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|Time")
	float WarmupTime = 15.f;

	float LevelStartedTime = 0.f;
	
protected:
	virtual void BeginPlay() override;
	
	void CalculateFurthestSpawnLocation(AActor*& OutSpawnPoint);
	
protected:
	UPROPERTY(EditAnywhere, Category = "Blaster|Gameplay")
	float KillScoreAward = 2.f;

private:
	float CountdownTime = 0.f;
};
