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
	virtual void PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABPlayerController* TargetBPC, ABPlayerController* AttackerBPC);
	virtual void RequestRespawn(ABlasterCharacter* CharacterToRespawn, AController* TargetBPC);

	virtual void OnPostLogin(AController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	
	void UpdatePlayerList();

public:
	TArray<TObjectPtr<AController>> LoginPlayerControllers;

	UPROPERTY()
	TArray<FPlayerStats> PlayerStats;

protected:
	UPROPERTY(EditAnywhere, Category = "Blaster|Gameplay")
	float KillScoreAward = 2.f;
	
	TObjectPtr<ABlasterGameState> BlasterGameState;
	
protected:
	void CalculateFurthestSpawnLocation(AActor*& OutSpawnPoint);
};
