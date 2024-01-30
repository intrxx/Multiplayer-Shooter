// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

class UBTeamSelect;
class UBKillFeed;
class UBAnnouncement;
class UBInventoryWidget;
class UBScoreBoard;
class UTexture2D;
class UBCharacterOverlay;
class UUserWidget;

UENUM(BlueprintType)
enum EB_CrosshairType : uint8
{
	ECT_None UMETA(DisplayName = "Initial Type"),
	ECT_Static UMETA(DisplayName = "Static"),
	ECT_Dynamic UMETA(DisplayName = "Dynamic"),
	ECT_DynamicOnlyShooting UMETA(DisplayName = "Dynamic - Only Shooting"),
	ECT_DynamicOnlyMovement UMETA(DisplayName = "Dynamic - Only Movement"),

	ECT_DefaultMAX UMETA(DisplayName = "Default MAX")
};

USTRUCT(BlueprintType)
struct FCrosshairInfo
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TObjectPtr<UTexture2D> CrosshairDot;

	UPROPERTY()
	TObjectPtr<UTexture2D> CrosshairLeft;

	UPROPERTY()
	TObjectPtr<UTexture2D> CrosshairRight;

	UPROPERTY()
	TObjectPtr<UTexture2D> CrosshairTop;

	UPROPERTY()
	TObjectPtr<UTexture2D> CrosshairBottom;

	float CrosshairSpread;
	EB_CrosshairType CrosshairType = EB_CrosshairType::ECT_Dynamic;
	bool bDrawCrosshairDot = true;
	bool bChangeColorOnEnemy = true;
	bool bShrinkOnEnemy = true;
	FLinearColor CrosshairColor = FLinearColor::Green;
};

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	/** Adds all the UI to the screen, called from PC */
	void AddHUD();
	void RemoveHUD(bool bRemoveOverlay, bool bRemoveScoreboard, bool bRemoveInventory);
	void AddAnnouncement();
	void AddKillFeed(const FString& KillerName, const FString& KilledName, UTexture2D* GunImage);
	void AddTeamSelect();
	
	bool IsScoreboardVisible();
	bool IsInventoryVisible();

	void SetHUDPackage(const FCrosshairInfo& Package){CrosshairInfo = Package;}
	void ToggleScoreboard(bool bIsVisible);
	void ToggleInventory(bool bIsVisible);

public:
	UPROPERTY()
	TObjectPtr<UBCharacterOverlay> CharacterOverlay;
	UPROPERTY()
	TObjectPtr<UBScoreBoard> Scoreboard;
	UPROPERTY()
	TObjectPtr<UBInventoryWidget> Inventory;
	UPROPERTY()
	TObjectPtr<UBAnnouncement> Announcement;
	UPROPERTY()
	TObjectPtr<UBKillFeed> KillFeed;
	UPROPERTY()
	UBTeamSelect* TeamSelect;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|UI")
	TSubclassOf<UUserWidget> CharacterOverlayClass;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|UI")
	TSubclassOf<UUserWidget> ScoreboardClass;

	UPROPERTY(EditAnywhere, Category = "Blaster|UI")
	TSubclassOf<UUserWidget> InventoryClass;

	UPROPERTY(EditAnywhere, Category = "Blaster|UI")
	TSubclassOf<UUserWidget> AnnouncementClass;

	UPROPERTY(EditAnywhere, Category = "Blaster|UI")
	TSubclassOf<UUserWidget> KillFeedClass;

	UPROPERTY(EditAnywhere, Category = "Blaster|UI")
	TSubclassOf<UUserWidget> TeamSelectClass;

	bool bHUDAdded = false;

protected:
	virtual void BeginPlay() override;

	void AddCharacterOverlay();
	void AddScoreBoard();
	void AddInventoryWidget();

protected:
	UPROPERTY()
	TArray<UBKillFeed*> KillFeedEntries;

private:
	void DrawCrosshairElement(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	UFUNCTION()
	void KillFeedEntryTimerFinished(UBKillFeed* KillFeedToRemove);
	
private:
	UPROPERTY()
	TObjectPtr<APlayerController> OwningPlayer;
	
	FCrosshairInfo CrosshairInfo;
	
	UPROPERTY(EditAnywhere, Category = "Blaster|HUD|Crosshair")
	float CrosshairSpreadMax = 16.f;

	UPROPERTY(EditAnywhere, Category = "Blaster|HUD|KillFeed")
	float KillFeedEntryTime = 3.5f;
	
};

