#pragma once

UENUM(BlueprintType)
enum class EBTeam : uint8
{
	EBT_NoTeam UMETA(DisplayName = "No Team"),
	EBT_RedTeam UMETA(DisplayName = "Red Team"),
	EBT_BlueTeam UMETA(DisplayName = "Blue Team"),

	EBT_MAX UMETA(DisplayName = "DefaultMAX")
};
