// Copyright 2026 out of sCope team - intrxx

#include "Game/BCaptureTheFlagGameMode.h"
#include "Weapon/BFlag.h"
#include "CaptureTheFlag/BFlagZone.h"
#include "Game/BlasterGameState.h"

void ABCaptureTheFlagGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABPlayerController* TargetBPC,
                                                ABPlayerController* AttackerBPC)
{
	ABlasterGameMode::PlayerEliminated(ElimmedCharacter, TargetBPC, AttackerBPC);
}

void ABCaptureTheFlagGameMode::FlagCaptured(const ABFlag* Flag, const ABFlagZone* FlagZone)
{
	bool bValidCapture = Flag->GetTeam() != FlagZone->ZoneTeam;
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(GameState);
	if(BGameState && bValidCapture)
	{
		if(FlagZone->ZoneTeam == EBTeam::EBT_BlueTeam)
		{
			BGameState->BlueTeamScores(2.f);
		}

		if(FlagZone->ZoneTeam == EBTeam::EBT_RedTeam)
		{
			BGameState->RedTeamScores(2.f);
		}
	}
}
