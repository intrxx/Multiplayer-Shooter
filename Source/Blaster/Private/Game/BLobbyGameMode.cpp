// Copyright 2026 out of sCope team - intrxx


#include "Game/BLobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "MultiplayerSessionsSubsystem.h"

void ABLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		UMultiplayerSessionsSubsystem* SessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(SessionsSubsystem);

		if(NumberOfPlayers == SessionsSubsystem->DesiredNumPublicConnections)
		{
			UWorld* World = GetWorld();
			if(World)
			{
				bUseSeamlessTravel = true;

				FString MatchType = SessionsSubsystem->DesiredMatchType;
				if(MatchType == "FreeForAll")
				{
					World->ServerTravel(FString("/Game/Blaster/Maps/BlasterMap?listen"));
				}

				//TODO Create Team Death Match map
				if(MatchType == "TeamDeathMatch")
				{
					World->ServerTravel(FString("/Game/Blaster/Maps/BlasterMap?listen"));
				}

				//TODO Create Capture The Flag map
				if(MatchType == "CaptureTheFlag")
				{
					World->ServerTravel(FString("/Game/Blaster/Maps/BlasterMap?listen"));
				}
			}
		}	
	}
}
