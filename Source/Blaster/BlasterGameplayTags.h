#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

class UGameplayTagsManager;

class BLASTER_API FBlasterGameplayTags
{
public:
	static const FBlasterGameplayTags& Get() {return GameplayTags;}
	static void InitializeNativeTags();

public:
	// Input
	FGameplayTag Input_Move;
	FGameplayTag Input_Look;
	FGameplayTag Input_Jump;
	FGameplayTag Input_EquipWeapon;

protected:
	void AddAllTags(UGameplayTagsManager& Manager);
	void AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment);
 
private:
	static FBlasterGameplayTags GameplayTags;
};
