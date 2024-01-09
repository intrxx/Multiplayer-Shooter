// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterComponents/BLagCompensationComponent.h"

#include "Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"

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

		ShowFramePackage(ThisFrame, FColor::Emerald);
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

void UBLagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn = HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComp() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetTail() == nullptr;
	
	if(bReturn)
	{
		return;
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
		return;
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
}

