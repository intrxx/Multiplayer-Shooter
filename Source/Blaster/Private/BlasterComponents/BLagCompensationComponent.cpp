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

	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.15f;
	
	if(CheckHeadShotForHit(HitCharacter, TraceStart, TraceEnd))
	{
		ResetHitBoxes(HitCharacter, CurrentFrame);
		return FBServerSideRewindResult{true, EBlasterBodyPart::BBP_Head};
	}
	
	if(CheckLegsForHit(HitCharacter, TraceStart, TraceEnd))
	{
		ResetHitBoxes(HitCharacter, CurrentFrame);
		return FBServerSideRewindResult{true, EBlasterBodyPart::BBP_Legs};
	}

	for(auto& HitBoxPair : HitCharacter->HitCollisionBoxesMap)
	{
		if(HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		}
	}

	UWorld* World = GetWorld();
	if(World)
	{
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility);

		if(ConfirmHitResult.bBlockingHit)
		{
			ResetHitBoxes(HitCharacter, CurrentFrame);
			return FBServerSideRewindResult{true, EBlasterBodyPart::BBP_Body};
		}
	}

	ResetHitBoxes(HitCharacter, CurrentFrame);
	return FBServerSideRewindResult{false, EBlasterBodyPart::BBP_None};
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

			return ConfirmHitResult.bBlockingHit;
		}
		return false;
	}
	return false;
}

bool UBLagCompensationComponent::CheckLegsForHit(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector& TraceEnd)
{
	if(HitCharacter == nullptr)
	{
		return false;
	}

	UBoxComponent* LeftUpLegBox = HitCharacter->HitCollisionBoxesMap[FName("LeftUpLeg")];
	UBoxComponent* LeftLegBox = HitCharacter->HitCollisionBoxesMap[FName("LeftLeg")];
	UBoxComponent* LeftFootBox = HitCharacter->HitCollisionBoxesMap[FName("LeftFoot")];
	
	UBoxComponent* RightUpLegBox = HitCharacter->HitCollisionBoxesMap[FName("RightUpLeg")];
	UBoxComponent* RightLegBox = HitCharacter->HitCollisionBoxesMap[FName("RightLeg")];
	UBoxComponent* RightFootBox = HitCharacter->HitCollisionBoxesMap[FName("RightFoot")];
	if(LeftUpLegBox && LeftLegBox && LeftFootBox && RightUpLegBox && RightLegBox && RightFootBox)
	{
		LeftUpLegBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		LeftUpLegBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		LeftLegBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		LeftLegBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		LeftFootBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		LeftFootBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

		RightUpLegBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		RightUpLegBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		RightLegBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		RightLegBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		RightFootBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		RightFootBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		
		FHitResult ConfirmHitResult;

		UWorld* World = GetWorld();
		if(World)
		{
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_Visibility);

			return ConfirmHitResult.bBlockingHit;
		}
		return false;
	}
	return false;
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
	bool bReturn = HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComp() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetTail() == nullptr;
	
	if(bReturn)
	{
		return FBServerSideRewindResult();
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
		return FBServerSideRewindResult();
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

	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

void UBLagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, ABWeapon* DamageCauser)
{
	FBServerSideRewindResult ConfirmHit = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if(BlasterCharacter && HitCharacter && DamageCauser && ConfirmHit.bHitConfirmed)
	{
		UGameplayStatics::ApplyDamage(HitCharacter, DamageCauser->GetDamage(),
			BlasterCharacter->Controller, DamageCauser, UDamageType::StaticClass());
	}
}

