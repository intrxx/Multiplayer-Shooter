// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/CombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/BlasterCharacter.h"
#include "Player/BPlayerController.h"
//#include "HUD/BlasterHUD.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/BWeapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 425.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if(BlasterCharacter)
	{
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if(BlasterCharacter->GetFollowCamera())
		{
			DefaultFOV = BlasterCharacter->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}

		CrosshairColor = CrosshairInfo.CrosshairColor;
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(BlasterCharacter && BlasterCharacter->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshair(HitResult, true);
		HitTarget = HitResult.ImpactPoint;
		
		SetHUDCrosshair(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::SetHUDCrosshair(float DeltaTime)
{
	if(BlasterCharacter == nullptr || BlasterCharacter->Controller == nullptr)
	{
		return;
	}

	BlasterPC = BlasterPC == nullptr ? Cast<ABPlayerController>(BlasterCharacter->Controller) : BlasterPC;
	if(BlasterPC)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(BlasterPC->GetHUD()) : BlasterHUD;
		if(BlasterHUD)
		{
			if(EquippedWeapon)
			{
				CrosshairInfo.CrosshairDot = EquippedWeapon->CrosshairDot;
				CrosshairInfo.CrosshairBottom = EquippedWeapon->CrosshairBottom;
				CrosshairInfo.CrosshairTop = EquippedWeapon->CrosshairTop;
				CrosshairInfo.CrosshairLeft = EquippedWeapon->CrosshairLeft;
				CrosshairInfo.CrosshairRight = EquippedWeapon->CrosshairRight;
			}
			else
			{
				CrosshairInfo.CrosshairDot = nullptr;
				CrosshairInfo.CrosshairBottom = nullptr;
				CrosshairInfo.CrosshairTop = nullptr;
				CrosshairInfo.CrosshairLeft = nullptr;
				CrosshairInfo.CrosshairRight = nullptr;
			}
			
			// Calculate crosshair spread
			// [0, 600] -> [0, 1]
			FVector2D WalkSpeedRange(0.f, BlasterCharacter->GetMovementComponent()->GetMaxSpeed());
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = BlasterCharacter->GetVelocity();
			Velocity.Z = 0.f;
			
			CrosshairMovementFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange,
				VelocityMultiplierRange,Velocity.Size());

			if(BlasterCharacter->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.2f, DeltaTime,
					2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime,
					25.f);
			}

			if(bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, -Combat::AimShrinkFactor, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 7.5f);
			
			if(CrosshairInfo.CrosshairType == EB_CrosshairType::ECT_Static)
			{
				CrosshairInfo.CrosshairSpread = 0.f;
			} // Static crosshair
			else if(CrosshairInfo.CrosshairType == EB_CrosshairType::ECT_Dynamic)
			{
				CrosshairInfo.CrosshairSpread =
					Combat::AimShrinkFactor +
					CrosshairMovementFactor +
					CrosshairInAirFactor +
					CrosshairAimFactor +
					CrosshairShootingFactor;
			} // Dynamic crosshair (Both Movement and Firing)
			else if(CrosshairInfo.CrosshairType == EB_CrosshairType::ECT_DynamicOnlyMovement)
			{
				CrosshairInfo.CrosshairSpread =
					CrosshairMovementFactor +
					CrosshairInAirFactor;
			} // Only Movement crosshair
			else if(CrosshairInfo.CrosshairType == EB_CrosshairType::ECT_DynamicOnlyShooting)
			{
				CrosshairInfo.CrosshairSpread =
					Combat::AimShrinkFactor +
					CrosshairAimFactor +
					CrosshairShootingFactor;
			} // Only Shooting crosshair
			
			BlasterHUD->SetHUDPackage(CrosshairInfo);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if(EquippedWeapon == nullptr)
	{
		return;
	}

	if(bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime,
			EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, UnZoomInterpSpeed);
	}

	if(BlasterCharacter && BlasterCharacter->GetFollowCamera())
	{
		BlasterCharacter->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::EquipWeapon(ABWeapon* WeaponToEquip)
{
	if(BlasterCharacter == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}

	//TODO Drop equipped weapon later if weapon is already equipped
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EBWeaponState::EWS_Equipped);
	
	const USkeletalMeshSocket* HandSocket = BlasterCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(WeaponToEquip, BlasterCharacter->GetMesh());
	}
	EquippedWeapon->SetOwner(BlasterCharacter);
	BlasterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	BlasterCharacter->bUseControllerRotationYaw = true;
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	// Leaving it here because if we call it on the client we don't need to wait for the ServerRPC to execute the function
	// so we see the result of clicking the aim button faster on the client and it then runs on the server replicating
	// to all clients so they can see the result too
	bAiming = bIsAiming;
	
	ServerSetAiming(bIsAiming);

	if(BlasterCharacter)
	{
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if(BlasterCharacter)
	{
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && BlasterCharacter)
	{
		BlasterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		BlasterCharacter->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	// Called locally
	bFireButtonPressed = bPressed;

	if(bFireButtonPressed)
	{
		// Called on client to run on server
		ServerFire(HitTarget);

		if(EquippedWeapon)
		{
			CrosshairShootingFactor += CrosshairShootingFactor + EquippedWeapon->GetShootingError(); 
			CrosshairShootingFactor = FMath::Clamp(CrosshairShootingFactor, EquippedWeapon->GetShootingError(),
				3.f);
		}
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	// Called on server
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon == nullptr)
	{
		return;
	}
	
	if(BlasterCharacter)
	{
		BlasterCharacter->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::TraceUnderCrosshair(FHitResult& OutHitResult, bool bUseDebug)
{
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// "UGameplayStatics::GetPlayerController(this, 0)" Even if this is a multiplayer game we can get the player with
	// index 0 as it is the player that plays the game on current machine
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(PC, CrosshairLocation,
		CrosshairWorldPosition, CrosshairWorldDirection);

	if(bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		if(BlasterCharacter)
		{
			float DistanceToCharacter = (BlasterCharacter->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 55.f);
			//Start.Y -= 50.f;
			//Start.Z -= 75.f;
			
			//DrawDebugSphere(GetWorld(), Start, 16.f, 12, FColor::Black, false);
		}
		FVector End = CrosshairWorldDirection * Combat::TraceLength;

		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECC_Visibility);
		
		if(OutHitResult.GetActor() && OutHitResult.GetActor()->Implements<UCrosshairInteractionInterface>())
		{
			if(CrosshairInfo.bChangeColorOnEnemy)
			{
				CrosshairInfo.CrosshairColor = FLinearColor::Red;
			}
		}
		else
		{
			CrosshairInfo.CrosshairColor = CrosshairColor;
		}

#if ENABLE_DRAW_DEBUG
		if(bUseDebug)
		{
			FColor DebugColor = OutHitResult.bBlockingHit ? FColor::Red : FColor::Green;
			
			DrawDebugSphere(GetWorld(), OutHitResult.ImpactPoint, 16.f, 16.f, DebugColor);
		}
#endif	// ENABLE_DRAW_DEBUG
	}
}







