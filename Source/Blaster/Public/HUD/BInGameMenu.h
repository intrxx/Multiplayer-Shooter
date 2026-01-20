// Copyright 2026 out of sCope team - intrxx

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BInGameMenu.generated.h"

class UButton;
class UMultiplayerSessionsSubsystem;

/**
 * 
 */
UCLASS()
class BLASTER_API UBInGameMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	void InGameMenuSetup();
	void InGameMenuTearDown();

protected:
	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	void OnPlayerLeftGame();

private:
	UFUNCTION()
	void ReturnButtonClicked();
	
	UFUNCTION()
	void MenuButtonClicked();

	UFUNCTION()
	void OptionsButtonClicked();
	
private:
	UPROPERTY()
	TObjectPtr<UMultiplayerSessionsSubsystem> MultiplayerSessionsSubsystem;

	UPROPERTY()
	TObjectPtr<APlayerController> PC;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> MenuButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> OptionsButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ReturnButton;
};
