// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameplayStatics.h"

void UBlasterGameplayStatics::BlasterDrawVectorDebugMessage(const FVector& Vector, float TimeToDisplay, FColor TextColor, const FString& VectorName)
{
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, TextColor,
			FString::Printf(TEXT("%s: x: %f y: %f z: %f"), *VectorName, Vector.X, Vector.Y, Vector.Z));
	}
}
