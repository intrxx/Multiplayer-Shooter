// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/CombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/BlasterCharacter.h"
#include "Player/BPlayerController.h"
#include "HUD/BlasterHUD.h"
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
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(BlasterCharacter && BlasterCharacter->IsLocallyControlled())
	{
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
			FHUDPackage HUDPackage;
			
			if(EquippedWeapon)
			{
				HUDPackage.CrosshairDot = EquippedWeapon->CrosshairDot;
				HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairBottom;
				HUDPackage.CrosshairTop = EquippedWeapon->CrosshairTop;
				HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight;
			}
			else
			{
				HUDPackage.CrosshairDot = nullptr;
				HUDPackage.CrosshairBottom = nullptr;
				HUDPackage.CrosshairTop = nullptr;
				HUDPackage.CrosshairLeft = nullptr;
				HUDPackage.CrosshairRight = nullptr;
			}
			// Calculate crosshair spread
			FCrosshairInfo CrosshairInfo = BlasterHUD->GetCrosshairInfo();

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
				HUDPackage.CrosshairSpread = 0.f;
			} // Static crosshair
			else if(CrosshairInfo.CrosshairType == EB_CrosshairType::ECT_Dynamic)
			{
				HUDPackage.CrosshairSpread =
					Combat::AimShrinkFactor +
					CrosshairMovementFactor +
					CrosshairInAirFactor +
					CrosshairAimFactor +
					CrosshairShootingFactor;
			} // Dynamic crosshair (Both Movement and Firing)
			else if(CrosshairInfo.CrosshairType == EB_CrosshairType::ECT_DynamicOnlyMovement)
			{
				HUDPackage.CrosshairSpread =
					CrosshairMovementFactor +
					CrosshairInAirFactor;
			} // Only Movement crosshair
			else if(CrosshairInfo.CrosshairType == EB_CrosshairType::ECT_DynamicOnlyShooting)
			{
				HUDPackage.CrosshairSpread =
					Combat::AimShrinkFactor +
					CrosshairAimFactor +
					CrosshairShootingFactor;
			} // Only Shooting crosshair
			
			BlasterHUD->SetHUDPackage(HUDPackage);
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
		FHitResult HitResult;
		TraceUnderCrosshair(HitResult, false);
		
		// Called on client to run on server
		ServerFire(HitResult.ImpactPoint);

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
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if(bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		FVector End = CrosshairWorldDirection * Combat::TraceLength;

		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECC_Visibility);

		if(!OutHitResult.bBlockingHit)
		{
			OutHitResult.ImpactPoint = End;
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







