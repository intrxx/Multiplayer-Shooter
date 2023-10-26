// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/Public/Character/BlasterCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Blaster/BlasterGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "BlasterComponents/CombatComponent.h"
#include "Character/BlasterAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Input/BlasterEnhancedInputComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/BPlayerController.h"
#include "Weapon/BWeapon.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArmComponent->SetupAttachment(GetMesh());
	SpringArmComponent->TargetArmLength = 450;
	SpringArmComponent->bUsePawnControlRotation = true;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadComponent"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponen"));
	CombatComp->SetIsReplicated(true);
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	TurningInPlace = EBTurningInPlace::ETIP_NotTurning;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 750.f, 0.f);

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(CombatComp)
	{
		CombatComp->BlasterCharacter = this;
	}
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (ABPlayerController* PC = Cast<ABPlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if(CombatComp && CombatComp->EquippedWeapon == nullptr)
	{
		return;
	}
	
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	
	if(!bIsInAir && Speed == 0.0f) // Standing still
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.0f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		bUseControllerRotationYaw = true;

		if(TurningInPlace == EBTurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		TurnInPlace(DeltaTime);
	}
	
	if(bIsInAir || Speed > 0.0f)
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.0f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = EBTurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	if(AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// Map pitch from the range [270, 360) to [-90, 0)
		const FVector2D InRange(270.f, 360.f);
		const FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if(AO_Yaw > 90.f) // Turning right
	{
		TurningInPlace = EBTurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw < -90.f) // Turning left
	{
		TurningInPlace = EBTurningInPlace::ETIP_Left;
	}
	
	if(TurningInPlace != EBTurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		
		if(FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = EBTurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.0f);
		}
	}
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UBlasterEnhancedInputComponent* BlasterInputComponent = Cast<UBlasterEnhancedInputComponent>(PlayerInputComponent);
	check(BlasterInputComponent)

	const FBlasterGameplayTags& GameplayTags = FBlasterGameplayTags::Get();

	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_Move, ETriggerEvent::Triggered, this,
		&ThisClass::Move);
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_Look, ETriggerEvent::Triggered, this,
		&ThisClass::Look);
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_Jump, ETriggerEvent::Triggered, this,
		&ThisClass::Jump);
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_EquipWeapon, ETriggerEvent::Triggered, this,
		&ThisClass::EquipButtonPressed);
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_Crouch, ETriggerEvent::Triggered, this,
		&ThisClass::CrouchButtonPressed);
	
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_Aim, ETriggerEvent::Started, this,
		&ThisClass::AimButtonPressed);
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_Aim, ETriggerEvent::Completed, this,
		&ThisClass::AimButtonReleased);
	
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_Fire, ETriggerEvent::Started, this,
		&ThisClass::FireWeaponPressed);
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_Fire, ETriggerEvent::Completed, this,
		&ThisClass::FireWeaponReleased);
}

void ABlasterCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D DirectionValue = Value.Get<FVector2D>();
	if(GetController())
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);

		const FVector ForwardVector = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(ForwardVector, DirectionValue.Y);

		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(RightDirection, DirectionValue.X);
	}
}

void ABlasterCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookValue = Value.Get<FVector2D>();

	if(GetController())
	{
		if(LookValue.X != 0.0f)
		{
			AddControllerYawInput(LookValue.X);
		}

		if(LookValue.Y != 0.0f)
		{
			AddControllerPitchInput(-LookValue.Y);
		}
	}
}

void ABlasterCharacter::EquipButtonPressed()
{
	if(CombatComp)
	{
		if(HasAuthority())
		{
			CombatComp->EquipWeapon(OverlappingWeapon);
		}
		else // We don't have authority so we call ServerRPC (A function that is called on client and executed on server)
		{
			ServerEquip();
		}
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if(CombatComp)
	{
		CombatComp->SetAiming(true);
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if(CombatComp)
	{
		CombatComp->SetAiming(false);
	}
}

void ABlasterCharacter::Jump()
{
	if(bIsCrouched)
	{
		UnCrouch();
		Super::Jump();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::FireWeaponPressed()
{
	if(CombatComp)
	{
		CombatComp->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireWeaponReleased()
{
	if(CombatComp)
	{
		CombatComp->FireButtonPressed(false);
	}
}

ABWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if(CombatComp == nullptr)
	{
		return nullptr;
	}
	return CombatComp->EquippedWeapon;
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (CombatComp && CombatComp->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (CombatComp && CombatComp->bAiming);
}

void ABlasterCharacter::SetOverlappingWeapon(ABWeapon* Weapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(false);
	}
	
	OverlappingWeapon = Weapon;
	if(IsLocallyControlled())
	{
		if(OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickUpWidget(true);
		}
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if(CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName = bAiming ? FName("FireRifleAim") : FName("FireRifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(ABWeapon* LastWeapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(true);
	}

	if(LastWeapon)
	{
		LastWeapon->ShowPickUpWidget(false);	
	}
}

void ABlasterCharacter::ServerEquip_Implementation()
{
	if(CombatComp)
	{
		CombatComp->EquipWeapon(OverlappingWeapon);
	}
}





