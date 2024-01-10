// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterComponents/BLagCompensationComponent.h"

#include "Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Weapon/BWeapon.h"

UBLagCompensationComponent::UBLagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}

void UBLagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UBLagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SavePackage();
}

void UBLagCompensationComponent::SavePackage()
{
	if(BlasterCharacter == nullptr || !BlasterCharacter->HasAuthority())
	{
		return;
	}
	
	if(FrameHistory.Num() <= 1)
	{
		FBFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		
		while(HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}

		FBFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);

		//ShowFramePackage(ThisFrame, FColor::Emerald);
	}
}

void UBLagCompensationComponent::SaveFramePackage(FBFramePackage& FramePackage)
{
	BlasterCharacter = BlasterCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterCharacter;
	if(BlasterCharacter)
	{
		FramePackage.Time = GetWorld()->GetTimeSeconds();
		FramePackage.Character = BlasterCharacter;
		
		for(auto& BoxPair : BlasterCharacter->HitCollisionBoxesMap)
		{
			FBBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();

			FramePackage.HitBoxInfoMap.Add(BoxPair.Key, BoxInformation);
		}
	}
}

void UBLagCompensationComponent::ShowFramePackage(const FBFramePackage& FramePackage, const FColor& Color)
{
	for(auto& BoxInfo : FramePackage.HitBoxInfoMap)
	{
		DrawDebugBox(GetWorld(), BoxInfo.Value.Location, BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation), Color, false, 4.f);
	}
}

FBFramePackage UBLagCompensationComponent::InterpBetweenFrames(const FBFramePackage& OlderFrame,
	const FBFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FBFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for(auto& YoungerPair : YoungerFrame.HitBoxInfoMap)
	{
		const FName& BoxInfoName = YoungerPair.Key;

		const FBBoxInformation& OlderBox = OlderFrame.HitBoxInfoMap[BoxInfoName];
		const FBBoxInformation& YoungerBox = YoungerFrame.HitBoxInfoMap[BoxInfoName];

		FBBoxInformation InterpBoxInfo;
		
		InterpBoxInfo.Location = FMath::Lerp(OlderBox.Location, YoungerBox.Location, InterpFraction);
		InterpBoxInfo.Rotation = FMath::Lerp(OlderBox.Rotation, YoungerBox.Rotation, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterpFramePackage.HitBoxInfoMap.Add(BoxInfoName, InterpBoxInfo);
	}
	
	return InterpFramePackage;
}

FBServerSideRewindResult UBLagCompensationComponent::ConfirmHit(const FBFramePackage& PackageToCheck,
	ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if(HitCharacter == nullptr)
	{
		return FBServerSideRewindResult();
	}

	FBFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	
	MoveBoxes(HitCharacter, PackageToCheck);
	
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.15f;
	
	if(CheckHeadShotForHit(HitCharacter, TraceStart, TraceEnd))
	{
		ResetHitBoxes(HitCharacter, CurrentFrame);
		UE_LOG(LogTemp, Warning, TEXT("Got a hitscan headshot!"));
		return FBServerSideRewindResult{true, EBlasterBodyPart::BBP_Head};
	}
	
	if(CheckLegsForHit(HitCharacter, TraceStart, TraceEnd))
	{
		ResetHitBoxes(HitCharacter, CurrentFrame);
		UE_LOG(LogTemp, Warning, TEXT("Got a hitscan legshot!"));
		return FBServerSideRewindResult{true, EBlasterBodyPart::BBP_Legs};
	}

	if(CheckBodyForHit(HitCharacter, TraceStart, TraceEnd))
	{
		ResetHitBoxes(HitCharacter, CurrentFrame);
		UE_LOG(LogTemp, Warning, TEXT("Got a hitscan bodyshot!"));
		return FBServerSideRewindResult{true, EBlasterBodyPart::BBP_Body};
	}

	UE_LOG(LogTemp, Warning, TEXT("Hitscan miss!"));
	ResetHitBoxes(HitCharacter, CurrentFrame);
	return FBServerSideRewindResult{false, EBlasterBodyPart::BBP_None};
}

FBShotgunSSRewindResult UBLagCompensationComponent::ShotgunConfirmHit(const TArray<FBFramePackage>& FramePackages,
	 const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	for(auto& Package : FramePackages)
	{
		if(Package.Character == nullptr)
		{
			return FBShotgunSSRewindResult();
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Confirming Hit"));

	FBShotgunSSRewindResult ShotgunSSRHitResult;

	TArray<FBFramePackage> CurrentFrames;
	for(auto& Package : FramePackages)
	{
		FBFramePackage CurrentFrame;
		CurrentFrame.Character = Package.Character;
		CacheBoxPositions(Package.Character, CurrentFrame);
		MoveBoxes(Package.Character, CurrentFrame);
		CurrentFrames.Add(CurrentFrame);
	}

	TArray<FVector> EndLocations;
	for(auto& HitLocation : HitLocations)
	{
		FVector EndLocation = TraceStart + (HitLocation - TraceStart) * 1.15f;
		EndLocations.Add(EndLocation);
	}
	
	// Adding to headshot count for every character hit
	for(auto& Package : FramePackages)
	{
		for(auto& EndLocation : EndLocations)
		{
			if(CheckHeadShotForHit(Package.Character, TraceStart, EndLocation))
			{
				if(ShotgunSSRHitResult.HeadShots.Contains(Package.Character))
				{
					ShotgunSSRHitResult.HeadShots[Package.Character]++;
					UE_LOG(LogTemp, Error, TEXT("increasing!"));
				}
				else
				{
					ShotgunSSRHitResult.HeadShots.Emplace(Package.Character, 1);
					UE_LOG(LogTemp, Error, TEXT("adding!"));
				}
				UE_LOG(LogTemp, Error, TEXT("Got a headshot!"));
			}
		}
	}

	// Adding to legshot count for every character hit
	for(auto& Package : FramePackages)
	{
		for(auto& EndLocation : EndLocations)
		{
			if(CheckLegsForHit(Package.Character, TraceStart, EndLocation))
			{
				if(ShotgunSSRHitResult.LegShots.Contains(Package.Character))
				{
					ShotgunSSRHitResult.LegShots[Package.Character]++;
				}
				else
				{
					ShotgunSSRHitResult.LegShots.Emplace(Package.Character, 1);
				}

				UE_LOG(LogTemp, Error, TEXT("Got a legshot!"));
			}
		}
	}

	// Adding to bodyshot count for every character hit
	for(auto& Package : FramePackages)
	{
		for(auto& EndLocation : EndLocations)
		{
			if(CheckLegsForHit(Package.Character, TraceStart, EndLocation))
			{
				if(ShotgunSSRHitResult.BodyShots.Contains(Package.Character))
				{
					ShotgunSSRHitResult.BodyShots[Package.Character]++;
				}
				else
				{
					ShotgunSSRHitResult.BodyShots.Emplace(Package.Character, 1);
				}
				UE_LOG(LogTemp, Error, TEXT("Got a bodyshot!"));
			}
		}
	}

	for(auto& Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.Character, Frame);
	}
	
	return ShotgunSSRHitResult;
}

bool UBLagCompensationComponent::CheckHeadShotForHit(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector& TraceEnd)
{
	if(HitCharacter == nullptr)
	{
		return false;
	}

	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxesMap[FName("Head")];
	if(HeadBox)
	{
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		
		FHitResult ConfirmHitResult;

		UWorld* World = GetWorld();
		if(World)
		{
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility);

			ABlasterCharacter* Character = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if(Character)
			{
				HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				return true;
			}
			HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			return false;
		}
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return false;
	}
	HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	return false;
}

bool UBLagCompensationComponent::CheckLegsForHit(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector& TraceEnd)
{
	if(HitCharacter == nullptr)
	{
		return false;
	}

	TArray<UBoxComponent*> LegHitBoxes;
	
	LegHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("LeftUpLeg")]);
	LegHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("LeftLeg")]);
	LegHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("LeftFoot")]);

	LegHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("RightUpLeg")]);
	LegHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("RightLeg")]);
	LegHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("RightFoot")]);

	for(auto& HitBox : LegHitBoxes)
	{
		if(HitBox)
		{
			HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			HitBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		}
	}
	
	FHitResult ConfirmHitResult;

	UWorld* World = GetWorld();
	if(World)
	{
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility);

		ABlasterCharacter* Character = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
		if(Character)
		{
			EnableBoxCollision(LegHitBoxes, ECollisionEnabled::NoCollision);
			return true;
		}
		EnableBoxCollision(LegHitBoxes, ECollisionEnabled::NoCollision);
		return false;
	}
	EnableBoxCollision(LegHitBoxes, ECollisionEnabled::NoCollision);
	return false;
}

bool UBLagCompensationComponent::CheckBodyForHit(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector& TraceEnd)
{
	if(HitCharacter == nullptr)
	{
		return false;
	}

	TArray<UBoxComponent*> BodyHitBoxes;
	
	BodyHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("Neck")]);
	BodyHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("Hips")]);
	BodyHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("Spine")]);
	BodyHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("Spine1")]);
	BodyHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("Spine2")]);
	
	BodyHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("LeftArm")]);
	BodyHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("LeftForeArm")]);
	BodyHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("LeftHand")]);
	BodyHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("RightArm")]);
	BodyHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("RightForeArm")]);
	BodyHitBoxes.Add(HitCharacter->HitCollisionBoxesMap[FName("RightHand")]);

	for(auto& HitBox : BodyHitBoxes)
	{
		if(HitBox)
		{
			HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			HitBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		}
	}
	
	FHitResult ConfirmHitResult;

	UWorld* World = GetWorld();
	if(World)
	{
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility);

		ABlasterCharacter* Character = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
		if(Character)
		{
			EnableBoxCollision(BodyHitBoxes, ECollisionEnabled::NoCollision);
			return true;
		}
		EnableBoxCollision(BodyHitBoxes, ECollisionEnabled::NoCollision);
		return false;
	}
	EnableBoxCollision(BodyHitBoxes, ECollisionEnabled::NoCollision);
	return false;
}

void UBLagCompensationComponent::EnableBoxCollision(const TArray<UBoxComponent*>& CollisionBoxes,
	ECollisionEnabled::Type CollisionEnabled)
{
	for(auto& HitBox : CollisionBoxes)
	{
		if(HitBox)
		{
			HitBox->SetCollisionEnabled(CollisionEnabled);
		}
	}
}

void UBLagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, /* OUT */ FBFramePackage& OutFramePackage)
{
	if(HitCharacter == nullptr)
	{
		return;
	}

	for(auto& HitBoxPair : HitCharacter->HitCollisionBoxesMap)
	{
		if(HitBoxPair.Value != nullptr)
		{
			FBBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();

			OutFramePackage.HitBoxInfoMap.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void UBLagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FBFramePackage& FramePackage)
{
	if(HitCharacter == nullptr)
	{
		return;
	}

	for(auto& HitBoxPair : HitCharacter->HitCollisionBoxesMap)
	{
		if(HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(FramePackage.HitBoxInfoMap[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(FramePackage.HitBoxInfoMap[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(FramePackage.HitBoxInfoMap[HitBoxPair.Key].BoxExtent);
		}
	}
}

void UBLagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FBFramePackage& FramePackage)
{
	if(HitCharacter == nullptr)
	{
		return;
	}

	for(auto& HitBoxPair : HitCharacter->HitCollisionBoxesMap)
	{
		if(HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(FramePackage.HitBoxInfoMap[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(FramePackage.HitBoxInfoMap[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(FramePackage.HitBoxInfoMap[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

FBServerSideRewindResult UBLagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter,
                                                  const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	return ConfirmHit(GetFrameToCheck(HitCharacter, HitTime), HitCharacter, TraceStart, HitLocation);
}

FBShotgunSSRewindResult UBLagCompensationComponent::ShotgunServerSideRewind(
	const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FBFramePackage> FramesToCheck;
	for(ABlasterCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}

	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

FBFramePackage UBLagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime)
{
	bool bReturn = HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComp() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetTail() == nullptr;
	
	if(bReturn)
	{
		return FBFramePackage();
	}

	// Frame package that we check to verify the hit
	FBFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	
	// Frame history of the hit character
	const TDoubleLinkedList<FBFramePackage>& History = HitCharacter->GetLagCompensationComp()->FrameHistory;
	
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	
	if(OldestHistoryTime > HitTime) // Too far back, too laggy to do SSR
	{
		return FBFramePackage();
	}

	if(OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	
	if(NewestHistoryTime <= HitTime) // We are probably server
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FBFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FBFramePackage>::TDoubleLinkedListNode* Older = Younger;

	while(Older->GetValue().Time > HitTime)
	{
		// March back until: OlderTime < HitTime < YoungerTime
		if(Older->GetNextNode() == nullptr)
		{
			break;
		}
		Older = Older->GetNextNode();

		if(Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}

	if(Older->GetValue().Time == HitTime) // Highly unlikely, but it can be our frame to check
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}

	if(bShouldInterpolate)
	{
		// Interpolate between Younger and Older because the HitTime is inbetween them
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}

	FrameToCheck.Character = HitCharacter;
	
	return FrameToCheck;
}

void UBLagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime, ABWeapon* DamageCauser)
{
	FBServerSideRewindResult ConfirmHit = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if(BlasterCharacter && HitCharacter && DamageCauser && ConfirmHit.bHitConfirmed)
	{
		UGameplayStatics::ApplyDamage(HitCharacter, DamageCauser->GetDamage(),
			BlasterCharacter->Controller, DamageCauser, UDamageType::StaticClass());
	}
}

void UBLagCompensationComponent::ServerShotgunScoreRequest_Implementation( const TArray<ABlasterCharacter*>& HitCharacters,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	UE_LOG(LogTemp, Warning, TEXT("HeyFromServerShotgunScoreRequest"));
	FBShotgunSSRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	if(HitCharacters.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Hit Characters Empty"));
	}
	
	for(auto& HitCharacter : HitCharacters)
	{
		if(BlasterCharacter == nullptr || HitCharacter == nullptr || BlasterCharacter->GetEquippedWeapon() == nullptr)
		{
			continue;
		}
		
		float TotalDamage = 0.f;
		
		if(Confirm.HeadShots.Contains(HitCharacter))
		{
			float HeadShotDamage = Confirm.HeadShots[HitCharacter] * BlasterCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += HeadShotDamage;
		}

		if(Confirm.HeadShots.Contains(HitCharacter))
		{
			float BodyShotDamage = Confirm.BodyShots[HitCharacter] * BlasterCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += BodyShotDamage;
		}

		if(Confirm.HeadShots.Contains(HitCharacter))
		{
			float LegShotDamage = Confirm.LegShots[HitCharacter] * BlasterCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += LegShotDamage;
		}

		UE_LOG(LogTemp, Error, TEXT("Damage: %f"), TotalDamage);
		
		UGameplayStatics::ApplyDamage(HitCharacter, TotalDamage,BlasterCharacter->Controller,
			BlasterCharacter->GetEquippedWeapon(), UDamageType::StaticClass());
	}
}

