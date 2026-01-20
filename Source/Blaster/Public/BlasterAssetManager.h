// Copyright 2026 out of sCope team - intrxx

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
