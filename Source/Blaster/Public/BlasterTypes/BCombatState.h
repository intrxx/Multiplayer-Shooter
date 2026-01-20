// Copyright 2026 out of sCope team - intrxx

#pragma once

UENUM(BlueprintType)
enum class EBCombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_ThrowingGrenade UMETA(DisplayName = "Throwing Grenade"),
	ECS_SwappingWeapon UMETA(DisplayName = "Swapping Weapon"),

	ECS_MAX UMETA(DisplayName = "Deafult MAX")
};
