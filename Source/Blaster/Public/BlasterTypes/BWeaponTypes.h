// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

namespace Combat
{
	static constexpr float TraceLength = 80000.0f;
	static constexpr float AimShrinkFactor = 0.35f;
	static constexpr float AimAtPlayerShrinkFactor = 0.25f;
	static constexpr float DamagePassedToShield = 0.66f;
	static constexpr float DamagePassedToHealth = 0.33f;
}

namespace BlasterStencil
{
	static constexpr int32 Purple = 250;
	static constexpr int32 Blue = 251;
	static constexpr int32 Red = 252;
}

UENUM(BlueprintType)
enum class EBWeaponType : uint8
{
	EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
	EWT_RocketLauncher UMETA(DisplayName = "Rocket Launcher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SubMachineGun UMETA(DisplayName = "Sub Machine Gun"),
	EWT_Shotgun UMETA(DisplayName = "Shotgun"),
	EWT_Sniper UMETA(DisplayName = "Sniper"),
	EWT_GrenadeLauncher UMETA(DisplayName = "Grenade Launcher"),
	
	EWT_MAX UMETA(DisplayName = "Default MAX")
};

UENUM(BlueprintType)
enum class EBGrenadeCategory : uint8
{
	EGC_None UMETA(DisplayName = "None"),
	EGC_Tactical UMETA(DisplayName = "Tactical"),
	EGC_Lethal UMETA(DisplayName = "Lethal"),
	
	EWC_MAX UMETA(DisplayName = "Default MAX")
};

UENUM(BlueprintType)
enum class EBGrenadeType : uint8
{
	EGT_Frag UMETA(DisplayName = "Frag"),
	EGT_Semtex UMETA(DisplayName = "Semtex"),
	EGT_Flash UMETA(DisplayName = "Flash"),
	
	EGT_MAX UMETA(DisplayName = "Default MAX")
};

