// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MultiplayerSessionsSubsystem.generated.h"

/*
 * Custom delegates created for the UMSMenuWidgetClass to bind callbacks to
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerSystemOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerSystemOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerSystemOnStartSessionComplete, bool, bWasSuccessful);

DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerSystemOnFindSessionComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerSystemOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplayerSessionsSubsystem();

	/*
	 * To handle session functionality. The Menu class will call these
	 */
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();

public:
	/*
	* Custom delegates created for the UMSMenuWidgetClass to bind callbacks to
	*/
	FMultiplayerSystemOnCreateSessionComplete MultiplayerSystemOnCreateSessionComplete;
	FMultiplayerSystemOnDestroySessionComplete MultiplayerSystemOnDestroySessionComplete;
	FMultiplayerSystemOnStartSessionComplete MultiplayerSystemOnStartSessionComplete;

	FMultiplayerSystemOnFindSessionComplete MultiplayerSystemOnFindSessionComplete;
	FMultiplayerSystemOnJoinSessionComplete MultiplayerSystemOnJoinSessionComplete;
	
protected:

	/*
	 * Internal callbacks for the delegates we will add to the Online Session Interface delegate list.
	 * These don't need to be called outside this class.
	 */
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	
private:
	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	bool bCreateSessionOnDestroy{false};
	int32 LastNumPublicConnections;
	FString LastMatchType;
	
	/*
	 * To add to the Online Session Interface delegate list.
	 * We will bind our MultiplayerSessionSubsystem internal callbacks to these.
	 */
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;

	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;
};
