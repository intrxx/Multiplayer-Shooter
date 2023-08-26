// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

#include "MSMenuWidgetClass.generated.h"

class UButton;
class UMultiplayerSessionsSubsystem;
/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMSMenuWidgetClass : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath  = FString(TEXT("/Game/TopDown/Maps/Lobby")));

	UPROPERTY(EditDefaultsOnly)
	bool bIsCursorEnabledInGame = false;
	
protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	/*
	 * Callbacks for the custom delegates on the MultiplayerSessionSubsystem
	 */
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

	void OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	
private:
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	UFUNCTION()
	void MenuTearDown();

	// The subsystem designed to handle all online session functionality
	UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	int32 NumPublicConnections{4};
	FString MatchType{TEXT("FreeForAll")};

	FString PathToLobby{TEXT("")};
	
};
