// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "HUD/BlasterHUD.h"
#include "BlasterGameMode.generated.h"

class ABlasterCharacter;
class ABPlayerController;
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

protected:
	UPROPERTY(EditAnywhere, Category = "Blaster|Gameplay")
	float KillScoreAward = 2.f;
	
	TArray<FPlayerStats> PlayerStats;
	
protected:
	void CalculateFurthestSpawnLocation(AActor*& OutSpawnPoint);
};
