// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "BlasterGameplayStatics.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterGameplayStatics : public UGameplayStatics
{
	GENERATED_BODY()

public:
	/**
	 * Custom function that handles drawing debug message with vector onto the screen
	 * @param Vector Vector that is drawn onto the screen
	 * @param TimeToDisplay Time to display the vector on screen
	 * @param TextColor Color of the drawn text
	 * @param VectorName Name of the vector displayed in text
	 */
	static void BlasterDrawVectorDebugMessage(const FVector& Vector, float TimeToDisplay, FColor TextColor,const  FString& VectorName = FString("Vector"));
};
