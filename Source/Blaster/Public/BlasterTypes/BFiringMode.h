// Fill out your copyright notice in the Description page of Project Settings.

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
