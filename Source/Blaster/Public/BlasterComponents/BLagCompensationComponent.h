// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BlasterTypes/BlasterBodyPart.h"
#include "BLagCompensationComponent.generated.h"

class ABPlayerController;
class ABlasterCharacter;
class ABWeapon;
class UBoxComponent;

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

	UPROPERTY()
	ABlasterCharacter* Character = nullptr;
};

USTRUCT()
struct FBServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed = false;

	UPROPERTY()
	EBlasterBodyPart BodyPartHit = EBlasterBodyPart::BBP_None;
};

USTRUCT()
struct FBShotgunSSRewindResult
{
	GENERATED_BODY()
	
	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> HeadShots;
	
	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> LegShots;
	
	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> BodyShots;
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
	
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation, float HitTime, ABWeapon* DamageCauser);

	UFUNCTION(Server, Reliable)
	void ServerShotgunScoreRequest(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations, float HitTime);
public:
	
protected:
	virtual void BeginPlay() override;

	FBServerSideRewindResult ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation, float HitTime);
	
	void SavePackage();
	void SaveFramePackage(FBFramePackage& FramePackage);
	FBFramePackage InterpBetweenFrames(const FBFramePackage& OlderFrame, const FBFramePackage& YoungerFrame, float HitTime);
	FBServerSideRewindResult ConfirmHit(const FBFramePackage& PackageToCheck, ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);
	
	void CacheBoxPositions(ABlasterCharacter* HitCharacter, /* OUT */ FBFramePackage& OutFramePackage);
	void MoveBoxes(ABlasterCharacter* HitCharacter, const FBFramePackage& FramePackage);
	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FBFramePackage& FramePackage);

	bool CheckHeadShotForHit(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector& TraceEnd);
	bool CheckLegsForHit(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector& TraceEnd);
	bool CheckBodyForHit(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector& TraceEnd);

	void EnableBoxCollision(const TArray<UBoxComponent*>& CollisionBoxes, ECollisionEnabled::Type CollisionEnabled);

	FBFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime);

	/**
	 * Shotgun
	 */
	FBShotgunSSRewindResult ShotgunServerSideRewind(const TArray<ABlasterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime);
	
	FBShotgunSSRewindResult ShotgunConfirmHit(const TArray<FBFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations);
private:
	UPROPERTY()
	TObjectPtr<ABlasterCharacter> BlasterCharacter;

	UPROPERTY()
	TObjectPtr<ABPlayerController> BlasterPC;

	TDoubleLinkedList<FBFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere, Category = "Blaster|ServerSideRewind")
	float MaxRecordTime = 4.f;
};
