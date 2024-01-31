#pragma once

UENUM(BlueprintType)
enum class EBTeam : uint8
{
	EBT_NoTeam UMETA(DisplayName = "No Team"),
	EBT_RedTeam UMETA(DisplayName = "Red Team"),
	EBT_BlueTeam UMETA(DisplayName = "Blue Team"),
	EBT_ChooseRandomTeam UMETA(DisplayName = "Choose Random Team"),

	EBT_MAX UMETA(DisplayName = "DefaultMAX")
};
