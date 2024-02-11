// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

#include "MSMenuWidgetClass.generated.h"

class UEditableTextBox;
class UCheckBox;
class UVerticalBox;
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
	void MenuSetup(int32 NumOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")),
		FString LobbyPath  = FString(TEXT("/Game/TopDown/Maps/Lobby")));

public:
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
	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	UFUNCTION()
	void ModesButtonClicked();

	UFUNCTION()
	void FFADeathMatchCheckStateChanged(bool bIsChecked);

	UFUNCTION()
	void TeamDeathMatchCheckStateChanged(bool bIsChecked);

	UFUNCTION()
	void CaptureTheFlagCheckStateChanged(bool bIsChecked);

	UFUNCTION()
	void PlayersCountTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void MenuTearDown();
	
private:
	// The subsystem designed to handle all online session functionality
	UPROPERTY()
	TObjectPtr<UMultiplayerSessionsSubsystem> MultiplayerSessionsSubsystem;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> HostButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> JoinButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ShowGameModesButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> ModesBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> FFADeathMatchCheckBox;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> TeamDeathMatchCheckBox;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> CaptureTheFlagCheckBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> NumOfPlayerTextBox;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> Appear;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> Hide;
	
	int32 NumPublicConnections{4};
	FString MatchType{TEXT("FreeForAll")};

	FString PathToLobby{TEXT("")};

	bool bIsModesVisible = false;
};
