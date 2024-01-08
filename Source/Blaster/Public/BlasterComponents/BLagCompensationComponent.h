// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BLagCompensationComponent.generated.h"

class ABPlayerController;
class ABlasterCharacter;

USTRUCT(BlueprintType)
struct FBBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location = FVector();

	UPROPERTY()
	FRotator Rotation = FRotator();

	UPROPERTY()
	FVector BoxExtent = FVector();
};

USTRUCT(BlueprintType)
struct FBFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time = 0;

	UPROPERTY()
	TMap<FName, FBBoxInformation> HitBoxInfoMap;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBLagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBLagCompensationComponent();
	friend ABlasterCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Debug function to show the hit box package on a single frame
	void ShowFramePackage(const FBFramePackage& FramePackage, const FColor& Color);

public:
	
protected:
	virtual void BeginPlay() override;

	void SaveFramePackage(FBFramePackage& FramePackage);

private:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> BlasterCharacter;

	UPROPERTY()
	TObjectPtr<ABPlayerController> BlasterPC;
};
