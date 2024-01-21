// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BPlayerState.h"
#include "Character/BlasterCharacter.h"
#include "Weapon/BWeapon.h"

void ABPlayerState::AddToScore(float ScoreToAdd)
{
	SetScore(GetScore() + ScoreToAdd);
}

void ABPlayerState::AddToDeaths(int32 DeathToAdd)
{
	Deaths += DeathToAdd;
}

void ABPlayerState::AddToKills(int32 KillsToAdd)
{
	Kills += KillsToAdd;
}

void ABPlayerState::AddToAssists(int32 AssistsToAdd)
{
	Assists += AssistsToAdd;
}


