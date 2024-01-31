// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/BScoreBoard.h"
#include "Components/VerticalBox.h"
#include "HUD/BScoreboardPlayerEntry.h"

void UBScoreBoard::UpdatePlayerList(TArray<FPlayerStats> PlayerStats)
{
	if(ScoreboardPlayerEntryClass == nullptr)
	{
		return;
	}
	
	PlayerContainer->ClearChildren();
	
	for(FPlayerStats Stats : PlayerStats)
	{
		//if(i > PlayerEntries.Num()-1)
	//	{
			UBScoreboardPlayerEntry* PlayerEntry = CreateWidget<UBScoreboardPlayerEntry>(this, ScoreboardPlayerEntryClass);
			if(PlayerEntry)
			{
				PlayerContainer->AddChild(PlayerEntry);
				PlayerEntries.Add(PlayerEntry);
				
				PlayerEntry->SetPlayerStats(Stats);
			}
	//	}

		//PlayerEntries[i]->SetPlayerName(PlayerStats[i]);
	//	PlayerContainer->AddChild(PlayerEntries[i]);
	}
}
