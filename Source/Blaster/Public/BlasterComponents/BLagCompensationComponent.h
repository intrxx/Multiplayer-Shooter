// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BlasterTypes/BlasterBodyPart.h"
#include "BLagCompensationComponent.generated.h"

class ABPlayerController;
class ABlasterCharacter;

USTRUCT(BlueprintType)
struct FBBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location = FVector::Zero();

	UPROPERTY()
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY()
	FVector BoxExtent = FVector::Zero();
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

USTRUCT()
struct FBServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	EBlasterBodyPart BodyPartHit = EBlasterBodyPart::BBP_None;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBLagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBLagCompensationComponent();
	friend ABlasterCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Debug function to show the hit box package 
	void ShowFramePackage(const FBFramePackage& FramePackage, const FColor& Color);
	FBServerSideRewindResult ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime);
public:
	
protected:
	virtual void BeginPlay() override;

	void SaveFramePackage(FBFramePackage& FramePackage);
	FBFramePackage InterpBetweenFrames(const FBFramePackage& OlderFrame, const FBFramePackage& YoungerFrame, float HitTime);
	FBServerSideRewindResult ConfirmHit(const FBFramePackage& PackageToCheck, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);
	void CacheBoxPositions(ABlasterCharacter* HitCharacter, /* OUT */ FBFramePackage& OutFramePackage);
	void MoveBoxes(ABlasterCharacter* HitCharacter, const FBFramePackage& FramePackage);
	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FBFramePackage& FramePackage);

	bool CheckHeadShotForHit(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector& TraceEnd);
	bool CheckLegsForHit(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector& TraceEnd);

private:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> BlasterCharacter;

	UPROPERTY()
	TObjectPtr<ABPlayerController> BlasterPC;

	TDoubleLinkedList<FBFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere, Category = "Blaster|ServerSideRewind")
	float MaxRecordTime = 1.f;
};
