// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

class UTexture2D;

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
struct FHUDPackage
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
};

USTRUCT(BlueprintType)
struct FCrosshairInfo
{
	GENERATED_BODY()
public:
	EB_CrosshairType CrosshairType = EB_CrosshairType::ECT_Dynamic;
	bool bDrawCrosshairDot = true;
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

	 void SetHUDPackage(const FHUDPackage& Package){HUDPackage = Package;}
	 FCrosshairInfo GetCrosshairInfo() const {return CrosshairInfo;}

private:
	void DrawCrosshairElement(UTexture2D* Texture, FVector2D ViewportCenter);
private:
	FHUDPackage HUDPackage;
	FCrosshairInfo CrosshairInfo;
};
