// Copyright 2026 out of sCope team - intrxx

#pragma once

UENUM(BlueprintType)
enum class EBFiringMode : uint8
{
	EFM_None UMETA(DisplayName = "None"),
	EFM_SingleBullet UMETA(DisplayName = "SingleBullet"),
	EFM_Burst UMETA(DisplayName = "Burst"),
	EFM_FullAuto UMETA(DisplayName = "FullAuto"),

	EFM_MAX UMETA(DisplayName = "Default MAX")
};
