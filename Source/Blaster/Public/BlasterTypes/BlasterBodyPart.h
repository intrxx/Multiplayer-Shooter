#pragma once

UENUM(BlueprintType)
enum class EBlasterBodyPart : uint8
{
	BBP_None UMETA(DisplayName = "None"),
	BBP_Head UMETA(DisplayName = "Head"),
	BBP_Body UMETA(DisplayName = "Body"),
	BBP_Legs UMETA(DisplayName = "Legs"),

	BBP_MAX UMETA(DisplayName = "Deafult MAX")
};
