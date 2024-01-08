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

