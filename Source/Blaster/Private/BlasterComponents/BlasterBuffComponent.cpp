// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/BlasterBuffComponent.h"

UBlasterBuffComponent::UBlasterBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UBlasterBuffComponent::BeginPlay()
{
	Super::BeginPlay();
	
	
}

void UBlasterBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	
}

