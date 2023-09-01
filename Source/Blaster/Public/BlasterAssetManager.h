// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "BlasterAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	UBlasterAssetManager();

	virtual void StartInitialLoading() override;
};
