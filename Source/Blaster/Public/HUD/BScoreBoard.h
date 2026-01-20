// Copyright 2026 out of sCope team - intrxx

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BScoreBoard.generated.h"

class UVerticalBox;
class UBScoreboardPlayerEntry;
struct FPlayerStats;
/**
 * 
 */
UCLASS()
class BLASTER_API UBScoreBoard : public UUserWidget
{
	GENERATED_BODY()

public:
	bool bIsVisible = false;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PlayerContainer;

public:
	void UpdatePlayerList(TArray<FPlayerStats> PlayerStats);

protected:
	TArray<TObjectPtr<UBScoreboardPlayerEntry>> PlayerEntries;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|UI")
	TSubclassOf<UBScoreboardPlayerEntry> ScoreboardPlayerEntryClass;
};
