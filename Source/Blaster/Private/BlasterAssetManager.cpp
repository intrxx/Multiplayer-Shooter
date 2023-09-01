// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAssetManager.h"

#include "Blaster/BlasterGameplayTags.h"

UBlasterAssetManager::UBlasterAssetManager()
{
}

void UBlasterAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FBlasterGameplayTags::InitializeNativeTags();
}
