// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BScoreboardPlayerEntry.h"
#include "Components/TextBlock.h"


void UBScoreboardPlayerEntry::SetPlayerStats(FPlayerStats Stats)
{
	PlayerStats = Stats;
	
	if(PlayerName)
	{
		PlayerName->SetText(FText::FromString(PlayerStats.PlayerName));
	}
	
	if(KillsAmount)
	{
		FString Kills = FString::Printf(TEXT("%d"), PlayerStats.Kills);
		KillsAmount->SetText(FText::FromString(Kills));
	}

	if(DeathsAmount)
	{
		FString Deaths = FString::Printf(TEXT("%d"), PlayerStats.Deaths);
		DeathsAmount->SetText(FText::FromString(Deaths));
	}

	if(AssistsAmount)
	{
		FString Assists = FString::Printf(TEXT("%d"), PlayerStats.Assists);
		AssistsAmount->SetText(FText::FromString(Assists));
	}

	if(DamageAmount)
	{
		FString Damage = FString::Printf(TEXT("%d"), FMath::FloorToInt(PlayerStats.Damage));
		DamageAmount->SetText(FText::FromString(Damage));
	}

	if(ScoreAmount)
	{
		FString Score = FString::Printf(TEXT("%d"), FMath::FloorToInt(PlayerStats.Score));
		ScoreAmount->SetText(FText::FromString(Score));
	}
	
}
