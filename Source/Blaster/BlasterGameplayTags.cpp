#include "BlasterGameplayTags.h"
#include "GameplayTagsManager.h"

FBlasterGameplayTags FBlasterGameplayTags::GameplayTags;

void FBlasterGameplayTags::InitializeNativeTags()
{
	UGameplayTagsManager& Manager = UGameplayTagsManager::Get();

	GameplayTags.AddAllTags(Manager);

	Manager.DoneAddingNativeTags();
}

void FBlasterGameplayTags::AddAllTags(UGameplayTagsManager& Manager)
{
	AddTag(Input_Move, "Input.Move", "Tag used for move input.");
	AddTag(Input_Look, "Input.Look", "Tag used for look input.");
	AddTag(Input_Jump, "Input.Jump", "Tag used for jump input");
	AddTag(Input_EquipWeapon, "Input.EquipWeapon", "Tag used for equip input");
	AddTag(Input_Crouch, "Input.Crouch", "Tag used for crouch input");
	AddTag(Input_Aim, "Input.Aim", "Tag used for aim input");
	AddTag(Input_Fire, "Input.Fire", "Tag used for fire input");
	AddTag(Input_Reload, "Input.Reload", "Tag used for reload input");
	AddTag(Input_ThrowTacGrenade, "Input.ThrowTacGrenade", "Tag used for tactical grenades input");
	AddTag(Input_ThrowLethalGrenade, "Input.ThrowLethalGrenade", "Tag used for lethal grenades input");
	AddTag(Input_ChangeFiringType, "Input.ChangeFiringType", "Tag used for change firing type input");
	AddTag(Input_ToggleScoreboard, "Input.ToggleScoreboard", "Tag used for toggling ScoreBoard input");
	AddTag(Input_ToggleInventory, "Input.ToggleInventory", "Tag used for toggling inventory input");
	AddTag(Input_SwapToPrimaryWeapon, "Input.SwapToPrimaryWeapon", "Tag used for swapping to primary weapon");
	AddTag(Input_ToggleInGameMenu, "Input.ToggleInGameMenu", "Tag used for toggling in game menu");
}

void FBlasterGameplayTags::AddTag(FGameplayTag& OutTag, const ANSICHAR* TagName, const ANSICHAR* TagComment)
{
	OutTag = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TagName), FString(TEXT("(Native) ")) + FString(TagComment));
}
