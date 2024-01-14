// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterComponents/BCombatComponent.h"
#include "Camera/CameraComponent.h"
#include "Character/BlasterCharacter.h"
#include "Player/BPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Weapon/BWeapon.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Weapon/BGrenade.h"
#include "Weapon/Shotgun/BShotgun.h"
#include "Weapon/Sniper/BSniperRifle.h"

UBCombatComponent::UBCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UBCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UBCombatComponent, EquippedLethalGrenade);
	DOREPLIFETIME(UBCombatComponent, EquippedTacticalGrenade);
	DOREPLIFETIME(UBCombatComponent, bAiming);
	DOREPLIFETIME(UBCombatComponent, CombatState);
	DOREPLIFETIME(UBCombatComponent, GrenadeTypeThrowing);
	DOREPLIFETIME_CONDITION(UBCombatComponent, CarriedTacticalGrenades, COND_OwnerOnly)
	DOREPLIFETIME_CONDITION(UBCombatComponent, CarriedLethalGrenades, COND_OwnerOnly)
	DOREPLIFETIME_CONDITION(UBCombatComponent, CarriedAmmo, COND_OwnerOnly);
}

void UBCombatComponent::BeginPlay()
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

		if(BlasterCharacter->HasAuthority())
		{
			InitializeCarriedAmmo();
			AddDefaultGrenades();
		}
	}
}

void UBCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(BlasterCharacter && BlasterCharacter->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshair(HitResult, false);
		HitTarget = HitResult.ImpactPoint;
		
		SetHUDCrosshair(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UBCombatComponent::SetHUDCrosshair(float DeltaTime)
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

			if(bAimingAtAnotherPlayer)
			{
				if(CrosshairInfo.bChangeColorOnEnemy)
				{
					CrosshairInfo.CrosshairColor = FLinearColor::Red;
				}
			
				if(CrosshairInfo.bShrinkOnEnemy)
				{
					CrosshairAimAtAnotherPlayerFactor = FMath::FInterpTo(CrosshairAimAtAnotherPlayerFactor,
						-Combat::AimAtPlayerShrinkFactor, DeltaTime, 30.f);
				}
			}
			else
			{
				CrosshairInfo.CrosshairColor = CrosshairColor;
			
				if(CrosshairInfo.bShrinkOnEnemy)
				{
					CrosshairAimAtAnotherPlayerFactor = FMath::FInterpTo(CrosshairAimAtAnotherPlayerFactor,
						0.f, DeltaTime, 30.f);
				}
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
				
				if(CrosshairInfo.bShrinkOnEnemy)
				{
					CrosshairInfo.CrosshairSpread +=
						Combat::AimAtPlayerShrinkFactor +
						CrosshairAimAtAnotherPlayerFactor;
				}
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
				
				if(CrosshairInfo.bShrinkOnEnemy)
				{
					CrosshairInfo.CrosshairSpread +=
						Combat::AimAtPlayerShrinkFactor +
						CrosshairAimAtAnotherPlayerFactor;
				}
			} // Only Shooting crosshair
			
			BlasterHUD->SetHUDPackage(CrosshairInfo);
		}
	}
}

void UBCombatComponent::InterpFOV(float DeltaTime)
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

void UBCombatComponent::AttachActorToHand(AActor* ActorToAttach, const FName SocketName)
{
	if(BlasterCharacter == nullptr || BlasterCharacter->GetMesh() == nullptr || ActorToAttach == nullptr)
	{
		return;
	}
	
	const USkeletalMeshSocket* HandSocket = BlasterCharacter->GetMesh()->GetSocketByName(SocketName);
	if(HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, BlasterCharacter->GetMesh());
	}
}

void UBCombatComponent::UpdateCarriedAmmo()
{
	if(EquippedWeapon == nullptr)
	{
		return;
	}
	
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	BlasterPC = BlasterPC == nullptr ? Cast<ABPlayerController>(BlasterCharacter->Controller) : BlasterPC;
	if(BlasterPC)
	{
		BlasterPC->SetHUDCarriedAmmo(CarriedAmmo);
		BlasterPC->SetHUDInventoryCarriedAmmo(EquippedWeapon->GetWeaponType(), CarriedAmmo);
		BlasterPC->SetHUDWeaponTypeText(EquippedWeapon->GetWeaponType());
	}
}

void UBCombatComponent::PlayEquipWeaponSound(ABWeapon* Weapon)
{
	if(Weapon && Weapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Weapon->EquipSound, BlasterCharacter->GetActorLocation());
	}
}

void UBCombatComponent::ReloadEmptyWeapon()
{
	if(EquippedWeapon && EquippedWeapon->IsMagEmpty())
	{
		Reload();
	}
}

void UBCombatComponent::AddDefaultGrenades()
{
	if(DefaultLethalGrenade)
	{
		EquippedLethalGrenade = DefaultLethalGrenade.GetDefaultObject();
		UpdateHUDGrenadeImage(EBGrenadeCategory::EGC_Lethal);
	}

	if(DefaultTacticalGrenade)
	{
		EquippedTacticalGrenade = DefaultTacticalGrenade.GetDefaultObject();
		UpdateHUDGrenadeImage(EBGrenadeCategory::EGC_Tactical);
	}
}

void UBCombatComponent::EquipWeapon(ABWeapon* WeaponToEquip)
{
	if(BlasterCharacter == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}

	if(CombatState != EBCombatState::ECS_Unoccupied)
	{
		return;
	}

	if(EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
	{
		AttachSecondaryWeapon(WeaponToEquip);
	}
	else
	{
		AttachPrimaryWeapon(WeaponToEquip);
	}
	
	BlasterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	BlasterCharacter->bUseControllerRotationYaw = true;
}

void UBCombatComponent::SwapWeapon()
{
	if(CombatState == EBCombatState::ECS_SwappingWeapon || CombatState == EBCombatState::ECS_ThrowingGrenade || BlasterCharacter == nullptr || !BlasterCharacter->HasAuthority())
	{
		return;
	}

	BlasterCharacter->PlaySwapMontage();
	BlasterCharacter->bFinishedSwapping = false;
	CombatState = EBCombatState::ECS_SwappingWeapon;
}

void UBCombatComponent::AttachPrimaryWeapon(ABWeapon* WeaponToEquip)
{
	if(WeaponToEquip == nullptr)
	{
		return;
	}
	
	// If we hold a weapon we should drop it first
	DropEquippedWeapon();
	
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EBWeaponState::EWS_Equipped);
	
	AttachActorToHand(EquippedWeapon, FName("RightHandSocket"));
	
	EquippedWeapon->SetOwner(BlasterCharacter);
	EquippedWeapon->SetHUDAmmo();
	EquippedWeapon->SetHUDAmmoImage();

	UpdateCarriedAmmo();
	PlayEquipWeaponSound(EquippedWeapon);
	ReloadEmptyWeapon();
}

void UBCombatComponent::AttachSecondaryWeapon(ABWeapon* WeaponToEquip)
{
	if(WeaponToEquip == nullptr)
	{
		return;
	}
	
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EBWeaponState::EWS_EquippedSecondary);
	
	AttachActorToHand(SecondaryWeapon, FName("SecondaryWeaponSocket"));
	PlayEquipWeaponSound(SecondaryWeapon);
	SecondaryWeapon->SetOwner(BlasterCharacter);
}

void UBCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon && BlasterCharacter)
	{
		EquippedWeapon->SetWeaponState(EBWeaponState::EWS_Equipped);
		
		AttachActorToHand(EquippedWeapon, FName("RightHandSocket"));

		BlasterPC = BlasterPC == nullptr ? Cast<ABPlayerController>(BlasterCharacter->Controller) : BlasterPC;
		if(BlasterPC)
		{
			BlasterPC->SetHUDWeaponTypeText(EquippedWeapon->GetWeaponType());
		}
		
		PlayEquipWeaponSound(EquippedWeapon);
		
		BlasterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		BlasterCharacter->bUseControllerRotationYaw = true;

		EquippedWeapon->SetHUDAmmo();
		EquippedWeapon->SetHUDAmmoImage();
	}
}

void UBCombatComponent::OnRep_SecondaryWeapon()
{
	if(SecondaryWeapon && BlasterCharacter)
	{
		SecondaryWeapon->SetWeaponState(EBWeaponState::EWS_EquippedSecondary);

		AttachActorToHand(SecondaryWeapon, FName("SecondaryWeaponSocket"));

		PlayEquipWeaponSound(SecondaryWeapon);
	}
}

void UBCombatComponent::OnRep_EquippedLethalGrenade()
{
	UpdateHUDGrenadeImage(EBGrenadeCategory::EGC_Lethal);
}

void UBCombatComponent::OnRep_EquippedTacticalGrenade()
{
	UpdateHUDGrenadeImage(EBGrenadeCategory::EGC_Tactical);
}

void UBCombatComponent::Reload()
{
	if(CarriedAmmo > 0 && CombatState == EBCombatState::ECS_Unoccupied && EquippedWeapon && EquippedWeapon->GetAmmo() != EquippedWeapon->GetMagCapacity() && !bLocallyReloading)
	{
		ServerReload();
		HandleReload();

		bLocallyReloading = true;
	}
}

int32 UBCombatComponent::CalculateAmountToReload()
{
	if(EquippedWeapon == nullptr)
	{
		return 0;
	}

	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 CarriedAmmoAmount = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 LeastToReload = FMath::Min(RoomInMag, CarriedAmmoAmount);
		return FMath::Clamp(RoomInMag, 0, LeastToReload);
	}
	return 0;
}

void UBCombatComponent::ThrowGrenade(const EBGrenadeCategory GrenadeCategory)
{
	if((GrenadeCategory == EBGrenadeCategory::EGC_Lethal && CarriedLethalGrenades == 0) || GrenadeCategory == EBGrenadeCategory::EGC_Tactical && CarriedTacticalGrenades == 0)
	{
		return;
	}
	if(CombatState != EBCombatState::ECS_Unoccupied || EquippedWeapon == nullptr)
	{
		return;
	}
	
	GrenadeTypeThrowing = GrenadeCategory;
	CombatState = EBCombatState::ECS_ThrowingGrenade;
	if(BlasterCharacter)
	{
		BlasterCharacter->PlayThrowGrenadeMontage(GrenadeCategory);
		if(EquippedWeapon)
		{
			const FName ThrowingSocket = EquippedWeapon->GetWeaponType() == EBWeaponType::EWT_Pistol ||
				EquippedWeapon->GetWeaponType() == EBWeaponType::EWT_SubMachineGun ? FName("SmallWeaponLeftHandSocket") : FName("LeftHandSocket");
			AttachActorToHand(EquippedWeapon, ThrowingSocket);

			switch (GrenadeCategory)
			{
			case EBGrenadeCategory::EGC_Lethal:
				ShowAttachedGrenade(true, EquippedLethalGrenade->GetGrenadeMesh());
				break;
			case EBGrenadeCategory::EGC_Tactical:
				ShowAttachedGrenade(true, EquippedTacticalGrenade->GetGrenadeMesh());
				break;
			default:
				break;
			}
		}
	}
	
	if(BlasterCharacter && !BlasterCharacter->HasAuthority())
	{
		ServerThrowGrenade(GrenadeCategory);
	}

	if(BlasterCharacter && BlasterCharacter->HasAuthority())
	{
		UpdateGrenadesValues(GrenadeCategory);
		UpdateHUDGrenades(GrenadeCategory);
	}
}

void UBCombatComponent::ServerThrowGrenade_Implementation(const EBGrenadeCategory GrenadeCategory)
{
	if((GrenadeCategory == EBGrenadeCategory::EGC_Lethal && CarriedLethalGrenades == 0) || GrenadeCategory == EBGrenadeCategory::EGC_Tactical && CarriedTacticalGrenades == 0)
	{
		return;
	}
	
	GrenadeTypeThrowing = GrenadeCategory;
	CombatState = EBCombatState::ECS_ThrowingGrenade;
	if(BlasterCharacter)
	{
		BlasterCharacter->PlayThrowGrenadeMontage(GrenadeCategory);
		AttachActorToHand(EquippedWeapon, FName("LeftHandSocket"));
		ShowAttachedGrenade(true, EquippedLethalGrenade->GetGrenadeMesh());
	}
	UpdateGrenadesValues(GrenadeCategory);
	UpdateHUDGrenades(GrenadeCategory);
}

void UBCombatComponent::UpdateHUDGrenades(const EBGrenadeCategory GrenadeCategory)
{
	BlasterPC = BlasterPC == nullptr ? Cast<ABPlayerController>(BlasterCharacter->Controller) : BlasterPC;
	if(BlasterPC)
	{
		switch (GrenadeCategory)
		{
		case EBGrenadeCategory::EGC_Lethal:
			BlasterPC->SetHUDGrenadesNumber(CarriedLethalGrenades, EBGrenadeCategory::EGC_Lethal);
			break;
		case EBGrenadeCategory::EGC_Tactical:
			BlasterPC->SetHUDGrenadesNumber(CarriedTacticalGrenades, EBGrenadeCategory::EGC_Tactical);
			break;
		default:
			break;
		}
		
	}
}

void UBCombatComponent::UpdateHUDGrenadeImage(const EBGrenadeCategory GrenadeCategory)
{
	BlasterPC = BlasterPC == nullptr ? Cast<ABPlayerController>(BlasterCharacter->Controller) : BlasterPC;
	if(BlasterPC)
	{
		switch (GrenadeCategory)
		{
		case EBGrenadeCategory::EGC_Lethal:
			if(EquippedLethalGrenade)
			{
				BlasterPC->SetHUDGrenadesImage(EquippedLethalGrenade->GrenadeHUDImage, EBGrenadeCategory::EGC_Lethal);
			}
			break;
		case EBGrenadeCategory::EGC_Tactical:
			if(EquippedTacticalGrenade)
			{
				BlasterPC->SetHUDGrenadesImage(EquippedTacticalGrenade->GrenadeHUDImage, EBGrenadeCategory::EGC_Tactical);
			}
			break;
		default:
			break;
		}
		
	}
}

void UBCombatComponent::ServerReload_Implementation()
{
	if(BlasterCharacter == nullptr)
	{
		return;
	}
	
	CombatState = EBCombatState::ECS_Reloading;
	
	if(!BlasterCharacter->IsLocallyControlled())
	{
		HandleReload();
	}
}

void UBCombatComponent::HandleReload()
{
	if(BlasterCharacter)
	{
		BlasterCharacter->PlayReloadMontage();
	}
}

void UBCombatComponent::UpdateAmmoValues()
{
	if(BlasterCharacter == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}
	
	int32 AmmoReloadAmount = CalculateAmountToReload();
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= AmmoReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	BlasterPC = BlasterPC == nullptr ? Cast<ABPlayerController>(BlasterCharacter->Controller) : BlasterPC;
	if(BlasterPC)
	{
		BlasterPC->SetHUDCarriedAmmo(CarriedAmmo);
		BlasterPC->SetHUDInventoryCarriedAmmo(EquippedWeapon->GetWeaponType(), CarriedAmmo);
	}
	
	EquippedWeapon->AddAmmo(AmmoReloadAmount);
}

void UBCombatComponent::UpdateShotgunAmmoValues()
{
	if(BlasterCharacter == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}
	
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	BlasterPC = BlasterPC == nullptr ? Cast<ABPlayerController>(BlasterCharacter->Controller) : BlasterPC;
	if(BlasterPC)
	{
		BlasterPC->SetHUDCarriedAmmo(CarriedAmmo);
		BlasterPC->SetHUDInventoryCarriedAmmo(EquippedWeapon->GetWeaponType(), CarriedAmmo);
	}
	
	EquippedWeapon->AddAmmo(1);
	bCanFire = true;
	
	if(EquippedWeapon->IsMagFull() || CarriedAmmo == 0)
	{
		JumpToShotGunMontageEnd();
	}
}

void UBCombatComponent::UpdateGrenadesValues(const EBGrenadeCategory GrenadeCategory)
{
	//TODO Update when different grenades will be an option
	
	switch (GrenadeCategory)
	{
	case EBGrenadeCategory::EGC_Lethal:
		if(EquippedLethalGrenade)
		{
			CarriedLethalGrenades = FMath::Clamp(CarriedLethalGrenades - 1, 0, EquippedLethalGrenade->GetMaxGrenades());
		}
		break;
	case EBGrenadeCategory::EGC_Tactical:
		if(EquippedTacticalGrenade)
		{
			CarriedTacticalGrenades = FMath::Clamp(CarriedTacticalGrenades -1, 0, EquippedTacticalGrenade->GetMaxGrenades());
		}
	default:
		break;
	}
}

void UBCombatComponent::JumpToShotGunMontageEnd()
{
	if(BlasterCharacter == nullptr)
	{
		return;
	}
	
	UAnimInstance* AnimInstance = BlasterCharacter->GetMesh()->GetAnimInstance();
	if(AnimInstance && BlasterCharacter->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UBCombatComponent::FinishReloading()
{
	if(BlasterCharacter == nullptr)
	{
		return;
	}
	
	if(BlasterCharacter->HasAuthority())
	{
		CombatState = EBCombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}

	bLocallyReloading = false;
	// This will be true only on the locally controlled character - we check if the fire button was pressed to continue
	// firing when reload finishes
	//f(bFireButtonPressed)
	//{
	//	Fire();
	//}
}

void UBCombatComponent::FinishSwapChangeState()
{
	if(BlasterCharacter == nullptr)
	{
		return;
	}
	
	if(BlasterCharacter->HasAuthority())
	{
		CombatState = EBCombatState::ECS_Unoccupied;
	}
	
	BlasterCharacter->bFinishedSwapping = true;
}

void UBCombatComponent::FinishSwapAttachWeapons()
{
	ABWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;
	
	EquippedWeapon->SetWeaponState(EBWeaponState::EWS_Equipped);
	AttachActorToHand(EquippedWeapon, FName("RightHandSocket"));
	EquippedWeapon->SetHUDAmmo();
	EquippedWeapon->SetHUDAmmoImage();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(EquippedWeapon);

	SecondaryWeapon->SetWeaponState(EBWeaponState::EWS_EquippedSecondary);
	AttachActorToHand(SecondaryWeapon, FName("SecondaryWeaponSocket"));
}

void UBCombatComponent::ShotgunShellReload()
{
	if(BlasterCharacter && BlasterCharacter->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}

void UBCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case EBCombatState::ECS_Reloading:
		if(BlasterCharacter && !BlasterCharacter->IsLocallyControlled())
		{
			HandleReload();
		}
		break;
	case EBCombatState::ECS_Unoccupied:
		if(bFireButtonPressed)
		{
			Fire();
		}
		break;
	case EBCombatState::ECS_ThrowingGrenade:
		if(BlasterCharacter && !BlasterCharacter->IsLocallyControlled())
		{
			BlasterCharacter->PlayThrowGrenadeMontage(GrenadeTypeThrowing);
			if(EquippedWeapon)
			{
				const FName ThrowingSocket = EquippedWeapon->GetWeaponType() == EBWeaponType::EWT_Pistol ||
					EquippedWeapon->GetWeaponType() == EBWeaponType::EWT_SubMachineGun ? FName("SmallWeaponLeftHandSocket") : FName("LeftHandSocket");
				AttachActorToHand(EquippedWeapon, ThrowingSocket);
			}
			switch (GrenadeTypeThrowing)
			{
			case EBGrenadeCategory::EGC_Lethal:
				ShowAttachedGrenade(true, EquippedLethalGrenade->GetGrenadeMesh());
				break;
			case EBGrenadeCategory::EGC_Tactical:
				ShowAttachedGrenade(true, EquippedTacticalGrenade->GetGrenadeMesh());
				break;
			default:
				break;
			}
		}
		break;
	case EBCombatState::ECS_SwappingWeapon:
		if(BlasterCharacter && !BlasterCharacter->IsLocallyControlled())
		{
			BlasterCharacter->PlaySwapMontage();
		}
		break;
	default:
		break;
	}
}

void UBCombatComponent::OnRep_Aiming()
{
	if(BlasterCharacter && BlasterCharacter->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
	}
}

void UBCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if(BlasterCharacter)
	{
		BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? BaseWalkSpeed * AimWalkSpeedMultiplier : BaseWalkSpeed;
	}
}

void UBCombatComponent::FireButtonPressed(bool bPressed)
{
	if(EquippedWeapon == nullptr)
	{
		return;
	}
	
	// Called locally
	bFireButtonPressed = bPressed;
	if(bFireButtonPressed)
	{
		if(EquippedWeapon->GetFiringMode() == EBFiringMode::EFM_SingleBullet || EquippedWeapon->GetFiringMode() == EBFiringMode::EFM_FullAuto)
		{
			Fire();
		}
		else if(EquippedWeapon->GetFiringMode() == EBFiringMode::EFM_Burst)
		{
			UE_LOG(LogTemp, Error, TEXT("Weapon %s has Burst mode avaible but no Burst Mode logic"), *EquippedWeapon->GetName());
		}
	}
}

void UBCombatComponent::ThrowGrenadeFinished()
{
	CombatState = EBCombatState::ECS_Unoccupied;

	AttachActorToHand(EquippedWeapon, FName("RightHandSocket"));
}

void UBCombatComponent::DropEquippedWeapon()
{
	if(EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}

void UBCombatComponent::PickupAmmo(EBWeaponType WeaponType, int32 AmmoToAdd)
{
	if(CarriedAmmoMap.Contains(WeaponType) && MaxCarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] += FMath::Clamp(AmmoToAdd, 0.f, MaxCarriedAmmoMap[WeaponType]);

		UpdateCarriedAmmo();
	}

	if(EquippedWeapon && EquippedWeapon->IsMagEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

bool UBCombatComponent::ShouldSwapWeapons()
{
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}

void UBCombatComponent::ShowAttachedGrenade(bool bShow, UStaticMesh* GrenadeMesh)
{
	if(BlasterCharacter && BlasterCharacter->GetAttachedGrenade())
	{
		if(bShow && GrenadeMesh)
		{
			BlasterCharacter->GetAttachedGrenade()->SetStaticMesh(GrenadeMesh);
		}
		BlasterCharacter->GetAttachedGrenade()->SetVisibility(bShow);
	}
}

void UBCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false);

	if(BlasterCharacter && BlasterCharacter->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UBCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if(BlasterCharacter && BlasterCharacter->GetAttachedGrenade())
	{
		const FVector StartingLocation = BlasterCharacter->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = BlasterCharacter;
		SpawnParameters.Instigator = BlasterCharacter;

		UWorld* World = GetWorld();
		if(World)
		{
			switch (GrenadeTypeThrowing)
			{
			case EBGrenadeCategory::EGC_Lethal:
				World->SpawnActor<ABGrenade>(EquippedLethalGrenade->GetClass(), StartingLocation, ToTarget.Rotation(), SpawnParameters);
				break;
			case EBGrenadeCategory::EGC_Tactical:
				World->SpawnActor<ABGrenade>(EquippedTacticalGrenade->GetClass(), StartingLocation, ToTarget.Rotation(), SpawnParameters);
				break;
			default:
				break;
			}
		}
	}
}

void UBCombatComponent::Fire()
{
	if(CanFire())
	{
		bCanFire = false;
		
		if(EquippedWeapon)
		{
			ShrinkCrosshairWhileShooting();

			switch (EquippedWeapon->FireType)
			{
			case EBFireType::EFT_ProjectileWeapon:
				FireProjectileWeapon();
				break;
			case EBFireType::EFT_HitScan:
				FireHitScanWeapon();
				break;
			case EBFireType::EFT_Shotgun:
				FireShotgun();
				break;
			default:
				break;
			}
		}
		
		StartFireTimer();
	}
	
	if(BlasterCharacter && EquippedWeapon && EquippedWeapon->IsMagEmpty() && EquippedWeapon->EmptyMagSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), EquippedWeapon->EmptyMagSound, BlasterCharacter->GetActorLocation());
	}
}

void UBCombatComponent::FireProjectileWeapon()
{
	if(EquippedWeapon)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;

		if(BlasterCharacter && !BlasterCharacter->HasAuthority())
		{
			LocalFire(HitTarget);
		}
		ServerFire(HitTarget);
	}
}

void UBCombatComponent::FireHitScanWeapon()
{
	if(EquippedWeapon)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;

		if(BlasterCharacter && !BlasterCharacter->HasAuthority())
		{
			LocalFire(HitTarget);
		}
		ServerFire(HitTarget);
	}
}

void UBCombatComponent::FireShotgun()
{
	if(ABShotgun* Shotgun = Cast<ABShotgun>(EquippedWeapon))
	{
		TArray<FVector_NetQuantize> HitTargets;
		Shotgun->ShotgunScatter(HitTarget, HitTargets);
		
		if(BlasterCharacter && !BlasterCharacter->HasAuthority())
		{
			LocalShotgunFire(HitTargets);
		}
		ServerShotgunFire(HitTargets);
	}
}

bool UBCombatComponent::CanFire()
{
	if(EquippedWeapon == nullptr)
	{
		return false;
	}
	
	if(!EquippedWeapon->IsMagEmpty() && bCanFire && CombatState == EBCombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EBWeaponType::EWT_Shotgun)
	{
		return true;
	}
	
	if(bLocallyReloading)
	{
		return false;
	}
	
	return !EquippedWeapon->IsMagEmpty() && bCanFire && CombatState == EBCombatState::ECS_Unoccupied;
}

void UBCombatComponent::StartFireTimer()
{
	if(EquippedWeapon == nullptr)
	{
		return;
	}

	BlasterCharacter->GetWorldTimerManager().SetTimer(FireTimer, this, &UBCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay);
}

void UBCombatComponent::FireTimerFinished()
{
	bCanFire = true;
	if(bFireButtonPressed && EquippedWeapon->GetFiringMode() == EBFiringMode::EFM_FullAuto)
	{
		Fire();
	}

	ReloadEmptyWeapon();
}

void UBCombatComponent::OnRep_CarriedAmmo()
{
	BlasterPC = BlasterPC == nullptr ? Cast<ABPlayerController>(BlasterCharacter->Controller) : BlasterPC;
	if(BlasterPC)
	{
		if(EquippedWeapon)
		{
			BlasterPC->SetHUDInventoryCarriedAmmo(EquippedWeapon->GetWeaponType(), CarriedAmmo);
		}
		BlasterPC->SetHUDCarriedAmmo(CarriedAmmo);
	}

	bool bJumpToShotgunEnd = CombatState == EBCombatState::ECS_Reloading &&
							EquippedWeapon != nullptr &&
							EquippedWeapon->GetWeaponType() == EBWeaponType::EWT_Shotgun &&
							CarriedAmmo == 0;
	if(bJumpToShotgunEnd)
	{
		JumpToShotGunMontageEnd();
	}
}

void UBCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EBWeaponType::EWT_AssaultRifle, StartingRifleAmmo);
	CarriedAmmoMap.Emplace(EBWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EBWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EBWeaponType::EWT_SubMachineGun, StartingSmgAmmo);
	CarriedAmmoMap.Emplace(EBWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EBWeaponType::EWT_Sniper, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EBWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}

void UBCombatComponent::OnRep_CarriedLethalGrenades()
{
	UpdateHUDGrenades(EBGrenadeCategory::EGC_Lethal);
}

void UBCombatComponent::OnRep_CarriedTacticalGrenades()
{
	UpdateHUDGrenades(EBGrenadeCategory::EGC_Tactical);
}

void UBCombatComponent::InitializeCarriedGrenades()
{
	CarriedGrenadesMap.Emplace(EBGrenadeType::EGT_Frag, StartingFragGrenades);
	CarriedGrenadesMap.Emplace(EBGrenadeType::EGT_Flash, StartingFlashGrenades);
	CarriedGrenadesMap.Emplace(EBGrenadeType::EGT_Semtex, StartingSemtexGrenades);
}

void UBCombatComponent::ShrinkCrosshairWhileShooting()
{
	CrosshairShootingFactor += CrosshairShootingFactor + EquippedWeapon->GetShootingError(); 
	CrosshairShootingFactor = FMath::Clamp(CrosshairShootingFactor, EquippedWeapon->GetShootingError(),
		3.f);
}

void UBCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	// Called on server
	MulticastFire(TraceHitTarget);
}

void UBCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(BlasterCharacter && BlasterCharacter->IsLocallyControlled() && !BlasterCharacter->HasAuthority())
	{
		return;
	}
	
	LocalFire(TraceHitTarget);
}

void UBCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	
	MulticastShotgunFire(TraceHitTargets);
}

void UBCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if(BlasterCharacter && BlasterCharacter->IsLocallyControlled() && !BlasterCharacter->HasAuthority())
	{
		return;
	}
	
	LocalShotgunFire(TraceHitTargets);
}

void UBCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon == nullptr)
	{
		return;
	}
	
	if(BlasterCharacter && CombatState == EBCombatState::ECS_Unoccupied)
	{
		BlasterCharacter->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UBCombatComponent::LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	ABShotgun* Shotgun = Cast<ABShotgun>(EquippedWeapon);
	if(Shotgun == nullptr)
	{
		return;
	}
	
	if(BlasterCharacter && CombatState == EBCombatState::ECS_Reloading || CombatState == EBCombatState::ECS_Unoccupied)
	{
		BlasterCharacter->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = EBCombatState::ECS_Unoccupied;
	}
}

void UBCombatComponent::SetAiming(bool bIsAiming)
{
	if(BlasterCharacter == nullptr || EquippedWeapon == nullptr)
	{
		return;
	}
	// Leaving it here because if we call it on the client we don't need to wait for the ServerRPC to execute the function
	// so we see the result of clicking the aim button faster on the client and it then runs on the server replicating
	// to all clients so they can see the result too
	bAiming = bIsAiming;
	
	ServerSetAiming(bIsAiming);
	
	BlasterCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? BaseWalkSpeed * AimWalkSpeedMultiplier : BaseWalkSpeed;
	
	if(EquippedWeapon->GetWeaponType() == EBWeaponType::EWT_Sniper)
	{
		if(BlasterCharacter->IsLocallyControlled())
		{
			BlasterCharacter->ShowScopeWidget(bIsAiming);
		}

		PlayScopeSounds(bIsAiming);
	}

	if(BlasterCharacter->IsLocallyControlled())
	{
		bAimButtonPressed = bIsAiming;
	}
}

void UBCombatComponent::PlayScopeSounds(bool bIsAiming)
{
	if(ABSniperRifle* Sniper = Cast<ABSniperRifle>(EquippedWeapon))
	{
		if(Sniper->ScopeInSound && Sniper->ScopeOutSound)
		{
			if(bIsAiming)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sniper->ScopeOutSound,
				BlasterCharacter->GetActorLocation(), BlasterCharacter->GetActorRotation(), 0.7f);
			}
			else
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sniper->ScopeInSound,
				BlasterCharacter->GetActorLocation(), BlasterCharacter->GetActorRotation(), 0.7f);
			}
				
		}
	}
}

void UBCombatComponent::TraceUnderCrosshair(FHitResult& OutHitResult, bool bUseDebug)
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
		}
		FVector End = Start + CrosshairWorldDirection * Combat::TraceLength;

		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECC_Visibility);
		
		if(OutHitResult.GetActor() && OutHitResult.GetActor()->Implements<UBCrosshairInteractionInterface>())
		{
			bAimingAtAnotherPlayer = true;
		}
		else
		{
			bAimingAtAnotherPlayer = false;
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








