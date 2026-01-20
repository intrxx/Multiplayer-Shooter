// Copyright 2026 out of sCope team - intrxx


#include "BlasterGameplayStatics.h"

void UBlasterGameplayStatics::BlasterDrawVectorDebugMessage(const FVector& Vector, float TimeToDisplay, FColor TextColor, const FString& VectorName)
{
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, TextColor,
			FString::Printf(TEXT("%s: x: %f; y: %f; z: %f;"), *VectorName, Vector.X, Vector.Y, Vector.Z));
	}
}
