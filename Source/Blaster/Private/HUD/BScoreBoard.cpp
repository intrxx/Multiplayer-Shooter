// Copyright 2026 out of sCope team - intrxx


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
