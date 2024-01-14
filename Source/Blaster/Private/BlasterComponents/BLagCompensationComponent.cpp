// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterComponents/BLagCompensationComponent.h"

#include "Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Physics/BlasterCollisionChannels.h"
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

bool UBLagCompensationComponent::SingleShotCheckHeadShotForHit(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
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
		HeadBox->SetCollisionResponseToChannel(ECC_ObjectChannel_HitBox, ECR_Block);
		
		FHitResult ConfirmHitResult;

		UWorld* World = GetWorld();
		if(World)
		{
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_ObjectChannel_HitBox);

			if(ConfirmHitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if(Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(),
						FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
				}
			}

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

bool UBLagCompensationComponent::SingleShotCheckLegsForHit(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
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
			HitBox->SetCollisionResponseToChannel(ECC_ObjectChannel_HitBox, ECR_Block);
		}
	}
	
	FHitResult ConfirmHitResult;

	UWorld* World = GetWorld();
	if(World)
	{
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_ObjectChannel_HitBox);
		
		if(ConfirmHitResult.Component.IsValid())
		{
			UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
			if(Box)
			{
				DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(),
					FQuat(Box->GetComponentRotation()), FColor::Green, false, 8.f);
			}
		}

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

bool UBLagCompensationComponent::SingleShotCheckBodyForHit(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
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
			HitBox->SetCollisionResponseToChannel(ECC_ObjectChannel_HitBox, ECR_Block);
		}
	}
	
	FHitResult ConfirmHitResult;

	UWorld* World = GetWorld();
	if(World)
	{
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_ObjectChannel_HitBox);

		if(ConfirmHitResult.Component.IsValid())
		{
			UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
			if(Box)
			{
				DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(),
					FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
			}
		}

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

bool UBLagCompensationComponent::ProjectileCheckHeadShotForHit(ABlasterCharacter* HitCharacter,
	const FPredictProjectilePathParams& PathParams)
{
	if(HitCharacter == nullptr)
	{
		return false;
	}
	
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxesMap[FName("Head")];
	if(HeadBox)
	{
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		HeadBox->SetCollisionResponseToChannel(ECC_ObjectChannel_HitBox, ECR_Block);

		FPredictProjectilePathResult PathResult;

		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		
		if(PathResult.HitResult.Component.IsValid())
		{
			UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
			if(Box)
			{
				DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(),
					FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
			}
		}

		ABlasterCharacter* Character = Cast<ABlasterCharacter>(PathResult.HitResult.GetActor());
		if(Character)
		{
			HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			return true;
		}
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return false;
	}
	return false;
}

bool UBLagCompensationComponent::ProjectileCheckBodyShotForHit(ABlasterCharacter* HitCharacter,
	const FPredictProjectilePathParams& PathParams)
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
			HitBox->SetCollisionResponseToChannel(ECC_ObjectChannel_HitBox, ECR_Block);
		}
	}

	FPredictProjectilePathResult PathResult;

	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		
	if(PathResult.HitResult.Component.IsValid())
	{
		UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
		if(Box)
		{
			DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(),
			FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
		}
	}

	ABlasterCharacter* Character = Cast<ABlasterCharacter>(PathResult.HitResult.GetActor());
	if(Character)
	{
		EnableBoxCollision(BodyHitBoxes, ECollisionEnabled::NoCollision);
		return true;
	}
	EnableBoxCollision(BodyHitBoxes, ECollisionEnabled::NoCollision);
	return false;
}

bool UBLagCompensationComponent::ProjectileCheckLegShotForHit(ABlasterCharacter* HitCharacter,
	const FPredictProjectilePathParams& PathParams)
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
			HitBox->SetCollisionResponseToChannel(ECC_ObjectChannel_HitBox, ECR_Block);
		}
	}
	
	FPredictProjectilePathResult PathResult;

	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		
	if(PathResult.HitResult.Component.IsValid())
	{
		UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
		if(Box)
		{
			DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(),
			FQuat(Box->GetComponentRotation()), FColor::Green, false, 8.f);
		}
	}

	ABlasterCharacter* Character = Cast<ABlasterCharacter>(PathResult.HitResult.GetActor());
	if(Character)
	{
		EnableBoxCollision(LegHitBoxes, ECollisionEnabled::NoCollision);
		return true;
	}
	EnableBoxCollision(LegHitBoxes, ECollisionEnabled::NoCollision);
	return false;
}

void UBLagCompensationComponent::EnableHeadShotBoxCollisions(const TArray<FBFramePackage>& FramePackages, ECollisionEnabled::Type CollisionEnabled,
	ECollisionResponse CollisionResponse)
{
	for(auto& Package : FramePackages)
	{
		if(Package.Character)
		{
			UBoxComponent* HeadBox = Package.Character->HitCollisionBoxesMap[FName("Head")];
			if(HeadBox)
			{
				HeadBox->SetCollisionEnabled(CollisionEnabled);
				HeadBox->SetCollisionResponseToChannel(ECC_ObjectChannel_HitBox, CollisionResponse);
			}
		}
	}
}

void UBLagCompensationComponent::EnableBodyShotBoxCollisions(const TArray<FBFramePackage>& FramePackages,
	ECollisionEnabled::Type CollisionEnabled, ECollisionResponse CollisionResponse)
{
	for(auto& Package : FramePackages)
	{
		if(Package.Character)
		{
			if(Package.Character == nullptr)
			{
				continue;
			}
			
			TArray<UBoxComponent*> BodyHitBoxes;
	
			BodyHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("Neck")]);
			BodyHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("Hips")]);
			BodyHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("Spine")]);
			BodyHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("Spine1")]);
			BodyHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("Spine2")]);
	
			BodyHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("LeftArm")]);
			BodyHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("LeftForeArm")]);
			BodyHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("LeftHand")]);
			BodyHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("RightArm")]);
			BodyHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("RightForeArm")]);
			BodyHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("RightHand")]);
			
			if(!BodyHitBoxes.IsEmpty())
			{
				for(auto& HitBox : BodyHitBoxes)
				{
					if(HitBox)
					{
						HitBox->SetCollisionEnabled(CollisionEnabled);
						HitBox->SetCollisionResponseToChannel(ECC_ObjectChannel_HitBox, CollisionResponse);
					}
				}
			}
		}
	}
}

void UBLagCompensationComponent::EnableLegShotBoxCollisions(const TArray<FBFramePackage>& FramePackages,
	ECollisionEnabled::Type CollisionEnabled, ECollisionResponse CollisionResponse)
{
	for(auto& Package : FramePackages)
	{
		if(Package.Character)
		{
			if(Package.Character == nullptr)
			{
				continue;
			}
			
			TArray<UBoxComponent*> LegHitBoxes;
	
			LegHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("LeftUpLeg")]);
			LegHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("LeftLeg")]);
			LegHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("LeftFoot")]);

			LegHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("RightUpLeg")]);
			LegHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("RightLeg")]);
			LegHitBoxes.Add(Package.Character->HitCollisionBoxesMap[FName("RightFoot")]);
			
			if(!LegHitBoxes.IsEmpty())
			{
				for(auto& HitBox : LegHitBoxes)
				{
					if(HitBox)
					{
						HitBox->SetCollisionEnabled(CollisionEnabled);
						HitBox->SetCollisionResponseToChannel(ECC_ObjectChannel_HitBox, CollisionResponse);
					}
				}
			}
		}
	}
}

void UBLagCompensationComponent::EnableBoxCollision(TArray<UBoxComponent*>& CollisionBoxes,
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

void UBLagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter,
	ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
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

FBServerSideRewindResult UBLagCompensationComponent::HitScanConfirmHit(const FBFramePackage& PackageToCheck,
	ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if(HitCharacter == nullptr)
	{
		return FBServerSideRewindResult();
	}

	FBFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	
	MoveBoxes(HitCharacter, PackageToCheck);
	
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.15f;
	
	if(SingleShotCheckHeadShotForHit(HitCharacter, TraceStart, TraceEnd))
	{
		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FBServerSideRewindResult{true, EBlasterBodyPart::BBP_Head};
	}
	
	if(SingleShotCheckLegsForHit(HitCharacter, TraceStart, TraceEnd))
	{
		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FBServerSideRewindResult{true, EBlasterBodyPart::BBP_Legs};
	}

	if(SingleShotCheckBodyForHit(HitCharacter, TraceStart, TraceEnd))
	{
		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FBServerSideRewindResult{true, EBlasterBodyPart::BBP_Body};
	}
	
	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
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

	UWorld* World = GetWorld();
	if(World == nullptr)
	{
		return FBShotgunSSRewindResult();
	}
	
	FBShotgunSSRewindResult ShotgunSSRHitResult;
	FHitResult ConfirmHitResult;

	TArray<FBFramePackage> CurrentFrames;
	for(auto& Package : FramePackages)
	{
		FBFramePackage CurrentFrame;
		CurrentFrame.Character = Package.Character;
		CacheBoxPositions(Package.Character, CurrentFrame);
		MoveBoxes(Package.Character, CurrentFrame);
		EnableCharacterMeshCollision(Package.Character, ECollisionEnabled::NoCollision);
		CurrentFrames.Add(CurrentFrame);
	}
	
	TArray<FVector> EndLocations;
	for(auto& HitLocation : HitLocations)
	{
		const FVector EndLocation = TraceStart + (HitLocation - TraceStart) * 1.25f;
		EndLocations.Add(EndLocation);
	}
	
	EnableHeadShotBoxCollisions(FramePackages, ECollisionEnabled::QueryOnly, ECR_Block);

	// Adding to headshot count
	for(auto& EndLocation : EndLocations)
	{
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, EndLocation, ECC_ObjectChannel_HitBox);

		ABlasterCharacter* Character = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
		if(Character)
		{
			if(ConfirmHitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if(Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(),
						FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
				}
			}
			
			if(ShotgunSSRHitResult.HeadShots.Contains(Character))
			{
				ShotgunSSRHitResult.HeadShots[Character]++;
			}
			else
			{
				ShotgunSSRHitResult.HeadShots.Emplace(Character, 1);
			}
		}
	}

	EnableHeadShotBoxCollisions(FramePackages, ECollisionEnabled::NoCollision, ECR_Ignore);

	EnableBodyShotBoxCollisions(FramePackages, ECollisionEnabled::QueryOnly, ECR_Block);

	//  Adding to bodyshots count
	for(auto& EndLocation : EndLocations)
	{
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, EndLocation, ECC_ObjectChannel_HitBox);

		ABlasterCharacter* Character = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
		if(Character)
		{
			if(ConfirmHitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if(Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(),
						FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
				}
			}
			
			if(ShotgunSSRHitResult.BodyShots.Contains(Character))
			{
				ShotgunSSRHitResult.BodyShots[Character]++;
			}
			else
			{
				ShotgunSSRHitResult.BodyShots.Emplace(Character, 1);
			}
		}
	}

	EnableBodyShotBoxCollisions(FramePackages, ECollisionEnabled::NoCollision, ECR_Ignore);

	EnableLegShotBoxCollisions(FramePackages, ECollisionEnabled::QueryOnly, ECR_Block);

	// Adding to bodyshot count for every character hit
	for(auto& EndLocation : EndLocations)
	{
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, EndLocation, ECC_ObjectChannel_HitBox);

		ABlasterCharacter* Character = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
		if(Character)
		{
			if(ConfirmHitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if(Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(),
						FQuat(Box->GetComponentRotation()), FColor::Green, false, 8.f);
				}
			}
			
			if(ShotgunSSRHitResult.LegShots.Contains(Character))
			{
				ShotgunSSRHitResult.LegShots[Character]++;
			}
			else
			{
				ShotgunSSRHitResult.LegShots.Emplace(Character, 1);
			}
		}
	}

	EnableLegShotBoxCollisions(FramePackages, ECollisionEnabled::NoCollision, ECR_Ignore);

	for(auto& Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
	}
	
	return ShotgunSSRHitResult;
}


FBServerSideRewindResult UBLagCompensationComponent::ProjectileConfirmHit(const FBFramePackage& PackageToCheck, ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	if(HitCharacter == nullptr)
	{
		return FBServerSideRewindResult();
	}

	FBFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
	
	MoveBoxes(HitCharacter, PackageToCheck);
	
	FPredictProjectilePathParams PathParams;
	
	PathParams.bTraceWithCollision = true;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.StartLocation = TraceStart;
	PathParams.SimFrequency = ProjectilePathPredictSimFrequency;
	PathParams.ProjectileRadius = 5.f;
	PathParams.TraceChannel = ECC_ObjectChannel_HitBox;
	PathParams.ActorsToIgnore.Add(GetOwner());
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;

	if(ProjectileCheckHeadShotForHit(HitCharacter, PathParams))
	{
		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FBServerSideRewindResult{true, EBlasterBodyPart::BBP_Head};
	}

	if(ProjectileCheckBodyShotForHit(HitCharacter, PathParams))
	{
		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FBServerSideRewindResult{true, EBlasterBodyPart::BBP_Body};
	}

	if(ProjectileCheckLegShotForHit(HitCharacter, PathParams))
	{
		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FBServerSideRewindResult{true, EBlasterBodyPart::BBP_Legs};
	}

	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FBServerSideRewindResult{false, EBlasterBodyPart::BBP_None};
}

FBServerSideRewindResult UBLagCompensationComponent::HitScanServerSideRewind(ABlasterCharacter* HitCharacter,
                                                                             const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	return HitScanConfirmHit(GetFrameToCheck(HitCharacter, HitTime), HitCharacter, TraceStart, HitLocation);
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

FBServerSideRewindResult UBLagCompensationComponent::ProjectileServerSideRewind(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	return ProjectileConfirmHit(GetFrameToCheck(HitCharacter, HitTime), HitCharacter, TraceStart, InitialVelocity, HitTime);
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

void UBLagCompensationComponent::ServerHitScanScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime, ABWeapon* DamageCauser)
{
	FBServerSideRewindResult ConfirmHit = HitScanServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if(BlasterCharacter && HitCharacter && DamageCauser && ConfirmHit.bHitConfirmed)
	{
		UGameplayStatics::ApplyDamage(HitCharacter, DamageCauser->GetDamage(),
			BlasterCharacter->Controller, DamageCauser, UDamageType::StaticClass());
	}
}

void UBLagCompensationComponent::ServerShotgunScoreRequest_Implementation( const TArray<ABlasterCharacter*>& HitCharacters,
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	FBShotgunSSRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);
	
	for(auto& HitCharacter : HitCharacters)
	{
		if(HitCharacter == nullptr || HitCharacter->GetEquippedWeapon() == nullptr || BlasterCharacter == nullptr)
		{
			continue;
		}
		
		float TotalDamage = 0.f;
		
		if(Confirm.HeadShots.Contains(HitCharacter))
		{
			float HeadShotDamage = Confirm.HeadShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += HeadShotDamage;
		}

		if(Confirm.BodyShots.Contains(HitCharacter))
		{
			float BodyShotDamage = Confirm.BodyShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += BodyShotDamage;
		}

		if(Confirm.LegShots.Contains(HitCharacter))
		{
			float LegShotDamage = Confirm.LegShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += LegShotDamage;
		}
		
		UGameplayStatics::ApplyDamage(HitCharacter, TotalDamage,BlasterCharacter->Controller,
			HitCharacter->GetEquippedWeapon(), UDamageType::StaticClass());
	}
}

void UBLagCompensationComponent::ServerProjectileScoreRequest_Implementation(ABlasterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FBServerSideRewindResult ConfirmHit = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);

	if(BlasterCharacter && HitCharacter && ConfirmHit.bHitConfirmed)
	{
		UGameplayStatics::ApplyDamage(HitCharacter, BlasterCharacter->GetEquippedWeapon()->GetDamage(),
			BlasterCharacter->Controller, BlasterCharacter->GetEquippedWeapon(), UDamageType::StaticClass());
	}
}



