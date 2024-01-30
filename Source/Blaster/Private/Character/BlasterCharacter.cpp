// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/Public/Character/BlasterCharacter.h"
#include "Physics/BlasterCollisionChannels.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Blaster/BlasterGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "BlasterComponents/BCombatComponent.h"
#include "BlasterComponents/BlasterBuffComponent.h"
#include "Character/BlasterAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Game/BlasterGameMode.h"
#include "Input/BlasterEnhancedInputComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/BPlayerController.h"
#include "Weapon/BWeapon.h"
#include "TimerManager.h"
#include "BlasterComponents/BLagCompensationComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Player/BPlayerState.h"
#include "BlasterTypes/BWeaponTypes.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Game/BlasterGameState.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArmComponent->SetupAttachment(GetMesh());
	SpringArmComponent->TargetArmLength = 450;
	SpringArmComponent->bUsePawnControlRotation = true;
	
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
	GetMesh()->SetCollisionObjectType(ECC_ObjectChannel_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadComponent"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComp = CreateDefaultSubobject<UBCombatComponent>(TEXT("CombatComponen"));
	CombatComp->SetIsReplicated(true);

	BuffComponent = CreateDefaultSubobject<UBlasterBuffComponent>(TEXT("BuffComponent"));
	BuffComponent->SetIsReplicated(true);

	LagCompensationComp = CreateDefaultSubobject<UBLagCompensationComponent>(TEXT("LagCompensationComponent"));
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	TurningInPlace = EBTurningInPlace::ETIP_NotTurning;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 750.f, 0.f);

	DissolveTimelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComp"));

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachedGrenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeRightHandSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/**
	 * Hit Boxes for server side rewind
	 */

	HeadBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Head"));
	HeadBox->SetupAttachment(GetMesh(), FName("Head"));
	HitCollisionBoxesMap.Add(FName("Head"), HeadBox);

	NeckBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Neck"));
	NeckBox->SetupAttachment(GetMesh(), FName("Neck"));
	HitCollisionBoxesMap.Add(FName("Neck"), NeckBox);

	HipsBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Hips"));
	HipsBox->SetupAttachment(GetMesh(), FName("Hips"));
	HitCollisionBoxesMap.Add(FName("Hips"), HipsBox);
	
	SpineBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine"));
	SpineBox->SetupAttachment(GetMesh(), FName("Spine"));
	HitCollisionBoxesMap.Add(FName("Spine"), SpineBox);

	Spine1Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine1"));
	Spine1Box->SetupAttachment(GetMesh(), FName("Spine1"));
	HitCollisionBoxesMap.Add(FName("Spine1"), Spine1Box);

	Spine2Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine2"));
	Spine2Box->SetupAttachment(GetMesh(), FName("Spine2"));
	HitCollisionBoxesMap.Add(FName("Spine2"), Spine2Box);

	LeftArmBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftArm"));
	LeftArmBox->SetupAttachment(GetMesh(), FName("LeftArm"));
	HitCollisionBoxesMap.Add(FName("LeftArm"), LeftArmBox);

	LeftForeArmBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftForeArm"));
	LeftForeArmBox->SetupAttachment(GetMesh(), FName("LeftForeArm"));
	HitCollisionBoxesMap.Add(FName("LeftForeArm"), LeftForeArmBox);

	LeftHandBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftHand"));
	LeftHandBox->SetupAttachment(GetMesh(), FName("LeftHand"));
	HitCollisionBoxesMap.Add(FName("LeftHand"), LeftHandBox);

	RightArmBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightArm"));
	RightArmBox->SetupAttachment(GetMesh(), FName("RightArm"));
	HitCollisionBoxesMap.Add(FName("RightArm"), RightArmBox);

	RightForeArmBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightForeArm"));
	RightForeArmBox->SetupAttachment(GetMesh(), FName("RightForeArm"));
	HitCollisionBoxesMap.Add(FName("RightForeArm"), RightForeArmBox);

	RightHandBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightHand"));
	RightHandBox->SetupAttachment(GetMesh(), FName("RightHand"));
	HitCollisionBoxesMap.Add(FName("RightHand"), RightHandBox);

	LeftUpLegBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftUpLeg"));
	LeftUpLegBox->SetupAttachment(GetMesh(), FName("LeftUpLeg"));
	HitCollisionBoxesMap.Add(FName("LeftUpLeg"), LeftUpLegBox);

	LeftLegBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftLeg"));
	LeftLegBox->SetupAttachment(GetMesh(), FName("LeftLeg"));
	HitCollisionBoxesMap.Add(FName("LeftLeg"), LeftLegBox);

	LeftFootBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftFoot"));
	LeftFootBox->SetupAttachment(GetMesh(), FName("LeftFoot"));
	HitCollisionBoxesMap.Add(FName("LeftFoot"), LeftFootBox);

	RightUpLegBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightUpLeg"));
	RightUpLegBox->SetupAttachment(GetMesh(), FName("RightUpLeg"));
	HitCollisionBoxesMap.Add(FName("RightUpLeg"), RightUpLegBox);

	RightLegBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightLeg"));
	RightLegBox->SetupAttachment(GetMesh(), FName("RightLeg"));
	HitCollisionBoxesMap.Add(FName("RightLeg"), RightLegBox);

	RightFootBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightFoot"));
	RightFootBox->SetupAttachment(GetMesh(), FName("RightFoot"));
	HitCollisionBoxesMap.Add(FName("RightFoot"), RightFootBox);

	for(auto& Box : HitCollisionBoxesMap)
	{
		if(Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_ObjectChannel_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_ObjectChannel_HitBox, ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
	
	/**
	 * Leg bone names 
	 */

	LegBoneNames.Add("LeftUpLeg");
	LegBoneNames.Add("LeftLeg");
	LegBoneNames.Add("LeftFoot");
	LegBoneNames.Add("LeftToeBase");
	LegBoneNames.Add("RightUpLeg");
	LegBoneNames.Add("RightLeg");
	LegBoneNames.Add("RightFoot");
	LegBoneNames.Add("RightToeBase");
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shield);
	DOREPLIFETIME(ABlasterCharacter, DeathBotSpawnZOffset);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(CombatComp)
	{
		CombatComp->BlasterCharacter = this;
	}
	
	if(BuffComponent)
	{
		BuffComponent->BlasterCharacter = this;
		BuffComponent->SetInitialSpeed(GetCharacterMovement()->MaxWalkSpeed,
			GetCharacterMovement()->MaxWalkSpeedCrouched);
		BuffComponent->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}

	if(LagCompensationComp)
	{
		LagCompensationComp->BlasterCharacter = this;
		if(Controller)
		{
			LagCompensationComp->BlasterPC = Cast<ABPlayerController>(Controller);
		}
	}
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	
	SimProxiesTurn();
	TimeSinceLastMovementRep = 0.f;
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	const ULocalPlayer* Player = (GEngine && GetWorld()) ? GEngine->GetFirstGamePlayer(GetWorld()) : nullptr;
	if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Player))
	{
		if(DefaultMappingContext && InventoryMappingContext && CooldownStateMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			GameplayMappingContexts.Add(DefaultMappingContext);
			
			Subsystem->AddMappingContext(InventoryMappingContext, 1);
			GameplayMappingContexts.Add(InventoryMappingContext);

			Subsystem->AddMappingContext(CooldownStateMappingContext, 2);
		}
	}
	
	if (ABPlayerController* PC = Cast<ABPlayerController>(GetController()))
	{
		//TODO Look here in case of some weird Death Screen Behaviour
		PC->SetDeathScreenVisibility(false);
	}
	
	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}

	if(AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
	
	EquipDefaultWeapon();
	
	UpdateHUDAmmo();
	UpdateHUDHealth();
	UpdateHUDShield();
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	
	HideCharacterIfCameraClose();
	PollInit();
}

void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if(bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = EBTurningInPlace::ETIP_NotTurning;
		return;
	}
	
	if(GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementRep += DeltaTime;
		if(TimeSinceLastMovementRep > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void ABlasterCharacter::PollInit()
{
	if(BlasterPS == nullptr)
	{
		BlasterPS = GetPlayerState<ABPlayerState>();
		if(BlasterPS)
		{
			BlasterPS->AddToScore(0.f);
			
			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(GetWorld()));
			if(BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(BlasterPS))
			{
				MulticastGainedTheLead();
			}
		}
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if(CombatComp && CombatComp->EquippedWeapon == nullptr)
	{
		return;
	}
	
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	
	if(!bIsInAir && Speed == 0.0f) // Standing still and jumping
	{
		bRotateRootBone = true;
		
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
		bRotateRootBone = false;
		
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.0f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = EBTurningInPlace::ETIP_NotTurning;
	}
	CalculateAO_Pitch();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if(AO_Pitch > 45.f && !IsLocallyControlled())
	{
		// Map pitch from the range [270, 360) to [-90, 0)
		const FVector2D InRange(270.f, 360.f);
		const FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	return Velocity.Size();
}

void ABlasterCharacter::SimProxiesTurn()
{
	if(CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr)
	{
		return;
	}
	bRotateRootBone = false;
	
	float Speed = CalculateSpeed();
	if(Speed > 0)
	{
		TurningInPlace = EBTurningInPlace::ETIP_NotTurning;
		return;
	}
	
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	
	if(FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if(ProxyYaw > TurnThreshold)
		{
			TurningInPlace = EBTurningInPlace::ETIP_Right;
		}
		else if(ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = EBTurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = EBTurningInPlace::ETIP_NotTurning;
		}
		return;
	}

	TurningInPlace = EBTurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if(AO_Yaw > 45.f) // Turning right
	{
		TurningInPlace = EBTurningInPlace::ETIP_Right;
	}
	else if(AO_Yaw < -45.f) // Turning left
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
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_Crouch, ETriggerEvent::Triggered, this,
		&ThisClass::CrouchButtonPressed);
	
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_EquipWeapon, ETriggerEvent::Triggered, this,
		&ThisClass::EquipButtonPressed);
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_ChangeFiringType, ETriggerEvent::Triggered, this,
		&ThisClass::ChangeFiringModeButtonPressed);
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_ToggleScoreboard, ETriggerEvent::Triggered, this,
		&ThisClass::ToggleScoreBoard);
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_Reload, ETriggerEvent::Triggered, this,
		&ThisClass::ReloadButtonPressed);
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_ToggleInventory, ETriggerEvent::Triggered, this,
		&ThisClass::ToggleInventory);
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_SwapToPrimaryWeapon, ETriggerEvent::Triggered, this,
		&ThisClass::SwapButtonPressed);
	
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_Aim, ETriggerEvent::Started, this,
		&ThisClass::AimButtonPressed);
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_Aim, ETriggerEvent::Completed, this,
		&ThisClass::AimButtonReleased);
	
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_Fire, ETriggerEvent::Started, this,
		&ThisClass::FireWeaponPressed);
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_Fire, ETriggerEvent::Completed, this,
		&ThisClass::FireWeaponReleased);
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_ThrowTacGrenade, ETriggerEvent::Triggered, this,
		&ThisClass::TacticalGrenadeButtonPressed);
	BlasterInputComponent->BindNativeAction(InputConfig, GameplayTags.Input_ThrowLethalGrenade, ETriggerEvent::Triggered, this,
		&ThisClass::LethalGrenadeButtonPressed);
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
		ServerEquip();
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

void ABlasterCharacter::ReloadButtonPressed()
{
	if(CombatComp)
	{
		CombatComp->Reload();
	}
}

void ABlasterCharacter::ChangeFiringModeButtonPressed()
{
	if(CombatComp && CombatComp->EquippedWeapon && CombatComp->EquippedWeapon->CanChangeFiringMode())
	{
		CombatComp->EquippedWeapon->ChangeFiringMode();
	}
}

void ABlasterCharacter::ToggleScoreBoard()
{
	BlasterPC = BlasterPC == nullptr ? Cast<ABPlayerController>(Controller) : BlasterPC;
	if(BlasterPC)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(BlasterPC->GetHUD()) : BlasterHUD;
		if(BlasterHUD)
		{
			BlasterHUD->ToggleScoreboard(BlasterHUD->IsScoreboardVisible());
		}
	}
}

void ABlasterCharacter::ToggleInventory()
{
	BlasterPC = BlasterPC == nullptr ? Cast<ABPlayerController>(Controller) : BlasterPC;
	if(BlasterPC)
	{
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(BlasterPC->GetHUD()) : BlasterHUD;
		if(BlasterHUD)
		{
			UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(BlasterPC->GetLocalPlayer());
			if(BlasterHUD->IsInventoryVisible())
			{
				BlasterPC->SetShowMouseCursor(false);
				if(InputSubsystem)
				{
					InputSubsystem->AddMappingContext(DefaultMappingContext, 0.f);
				}
			}
			else
			{
				BlasterPC->SetShowMouseCursor(true);
				if(InputSubsystem)
				{
					InputSubsystem->RemoveMappingContext(DefaultMappingContext);
				}
			}
			BlasterHUD->ToggleInventory(BlasterHUD->IsInventoryVisible());
		}
	}
}

void ABlasterCharacter::LethalGrenadeButtonPressed()
{
	if(CombatComp)
	{
		CombatComp->ThrowGrenade(EBGrenadeCategory::EGC_Lethal);
	}
}

void ABlasterCharacter::TacticalGrenadeButtonPressed()
{
	if(CombatComp)
	{
		CombatComp->ThrowGrenade(EBGrenadeCategory::EGC_Tactical);
	}
}

void ABlasterCharacter::SwapButtonPressed()
{
	if(CombatComp)
	{
		if(CombatComp->bLocallyReloading)
		{
			CombatComp->bLocallyReloading = false;
		}
		
		if(CombatComp->CombatState != EBCombatState::ECS_SwappingWeapon && CombatComp->CombatState != EBCombatState::ECS_ThrowingGrenade)
		{
			ServerSwapButtonPressed();
		}
		
		if(CombatComp->ShouldSwapWeapons() && !HasAuthority())
		{
			if(GetMesh() && GetMesh()->GetAnimInstance() && !GetMesh()->GetAnimInstance()->Montage_IsPlaying(SwapWeaponMontage))
			{
				PlaySwapMontage();
				CombatComp->CombatState = EBCombatState::ECS_SwappingWeapon;
				bFinishedSwapping = false;
			}
		}
	}
}

void ABlasterCharacter::ServerSwapButtonPressed_Implementation()
{
	if(CombatComp && CombatComp->ShouldSwapWeapons())
	{
		CombatComp->SwapWeapon();
	}
}

EBCombatState ABlasterCharacter::GetCombatState() const
{
	if(CombatComp)
	{
		return CombatComp->CombatState;
	}
	return EBCombatState::ECS_MAX;
}

ABWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if(CombatComp == nullptr)
	{
		return nullptr;
	}
	return CombatComp->EquippedWeapon;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if(CombatComp == nullptr)
	{
		return FVector::ZeroVector;
	}
	return CombatComp->HitTarget;
}

bool ABlasterCharacter::IsLocallyReloading()
{
	if(CombatComp == nullptr)
	{
		return false;
	}

	return CombatComp->bLocallyReloading;
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (CombatComp && CombatComp->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (CombatComp && CombatComp->bAiming);
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if(DeathBotEffectComp)
	{
		DeathBotEffectComp->DestroyComponent();
	}

	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;
	
	if(CombatComp && CombatComp->EquippedWeapon && bMatchNotInProgress)
	{
		CombatComp->EquippedWeapon->Destroy();
	}
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

void ABlasterCharacter::PlayDeathMontage(bool bAiming)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if(AnimInstance == nullptr)
	{
		return;
	}
	
	if(FMath::RandRange(0, 100) < RareDeathChance)
	{
		if(RareDeathMontage)
		{
			DeathBotSpawnZOffset = 150.f;
			AnimInstance->Montage_Play(RareDeathMontage);
			return;
		}
	}

	DeathBotSpawnZOffset = 115.f;
	UAnimMontage* AnimMontageToPlay =
	bAiming ? RegularDeathMontages_Aim[FMath::RandRange(0,RegularDeathMontages_Aim.Num()-1)] :
	RegularDeathMontages_Hip[FMath::RandRange(0,RegularDeathMontages_Hip.Num()-1)];
	
	AnimInstance->Montage_Play(AnimMontageToPlay);
}

void ABlasterCharacter::PlayReloadMontage()
{
	if(CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && ReloadWeaponMontage)
	{
		AnimInstance->Montage_Play(ReloadWeaponMontage);
		switch (CombatComp->EquippedWeapon->GetWeaponType())
		{
		case EBWeaponType::EWT_AssaultRifle:
			AnimInstance->Montage_JumpToSection(TEXT("Rifle"));
			break;
		case EBWeaponType::EWT_RocketLauncher:
			AnimInstance->Montage_JumpToSection(TEXT("Rocket")); 
			break;
		case EBWeaponType::EWT_Pistol:
			AnimInstance->Montage_JumpToSection(TEXT("Pistol")); 
			break;
		case EBWeaponType::EWT_SubMachineGun:
			AnimInstance->Montage_JumpToSection(TEXT("Rifle")); // Don't have EWT_SubMachineGun Reload Anim right now
			break;
		case EBWeaponType::EWT_Shotgun:
			AnimInstance->Montage_JumpToSection(TEXT("Shotgun")); 
			break;
		case EBWeaponType::EWT_Sniper:
			AnimInstance->Montage_JumpToSection(TEXT("Sniper")); 
			break;
		case EBWeaponType::EWT_GrenadeLauncher:
			AnimInstance->Montage_JumpToSection(TEXT("GrenadeLauncher"));
			break;
		default:
			break;
		}
		
	}
}

void ABlasterCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && SwapWeaponMontage)
	{
		AnimInstance->Montage_Play(SwapWeaponMontage);
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage(const EBGrenadeCategory GrenadeCategory)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(!AnimInstance)
	{
		return;
	}
	AnimInstance->Montage_Play(ThrowGrenadeMontage);
	
	switch(GrenadeCategory)
	{
		case EBGrenadeCategory::EGC_Lethal:
			AnimInstance->Montage_JumpToSection(FName("Lethal"));
			break;
		case EBGrenadeCategory::EGC_Tactical:
			AnimInstance->Montage_JumpToSection(FName("Tactical"));
			break;
		default:
			break;
	}
}

void ABlasterCharacter::EquipDefaultWeapon()
{
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));

	UWorld* World = GetWorld();
	
	// Game modes are only on the server if we pass the check we know we are in a map with BlasterGameMode and on a server
	if(BlasterGameMode && World && !bDead && DefaultWeaponClass)
	{
		ABWeapon* StartingWeapon = World->SpawnActor<ABWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeaponOnDeath = true;
		
		if(CombatComp)
		{
			CombatComp->EquipWeapon(StartingWeapon);
		}
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if(CombatComp == nullptr || CombatComp->EquippedWeapon == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName = FName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
                                      AController* InstigatorController, AActor* DamageCauser)
{
	if(bDead)
	{
		return;
	}

	float DamageToHealth = Damage;
	
	if(Shield > 0)
	{
		float DamageToShield = FMath::RoundToFloat(Damage * Combat::DamagePassedToShield);
		float DamageToHealthThroughShield = FMath::RoundToFloat(Damage * Combat::DamagePassedToHealth);
		
		if(Shield >= DamageToShield)
		{
			Shield = FMath::Clamp(Shield - DamageToShield, 0.f, MaxShield);
			DamageToHealth = DamageToHealthThroughShield;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToShield - Shield, 0.f, DamageToShield) + DamageToHealthThroughShield;
			Shield = 0.f;
		}
	}
	
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
	
	UpdateHUDHealth();
	UpdateHUDShield();
	
	PlayHitReactMontage();

	if(Health == 0.f)
	{
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if(BlasterGameMode)
		{
			BlasterPC = BlasterPC == nullptr ? Cast<ABPlayerController>(Controller) : BlasterPC;
			ABPlayerController* AttackerController = Cast<ABPlayerController>(InstigatorController);
			
			BlasterGameMode->PlayerEliminated(this, BlasterPC, AttackerController);
		}
	}
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();

	if(Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();

	if(Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPC =  BlasterPC == nullptr ? Cast<ABPlayerController>(Controller) : BlasterPC;
	if(BlasterPC)
	{
		BlasterPC->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::UpdateHUDShield()
{
	BlasterPC =  BlasterPC == nullptr ? Cast<ABPlayerController>(Controller) : BlasterPC;
	if(BlasterPC)
	{
		BlasterPC->SetHUDShield(Shield, MaxShield);
	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPC =  BlasterPC == nullptr ? Cast<ABPlayerController>(Controller) : BlasterPC;
	if(BlasterPC && CombatComp && CombatComp->EquippedWeapon)
	{
		BlasterPC->SetHUDCarriedAmmo(CombatComp->CarriedAmmo);
		BlasterPC->SetHUDWeaponAmmo(CombatComp->EquippedWeapon->GetAmmo());
		BlasterPC->SetHUDWeaponAmmoImage(CombatComp->EquippedWeapon->GetFiringMode());
		BlasterPC->SetHUDInventoryCarriedAmmo(CombatComp->EquippedWeapon->GetWeaponType(), CombatComp->CarriedAmmo);
	}
}

void ABlasterCharacter::HandleDeath(bool bPlayerLeftGame)
{
	// Drop or destroy all held weapons
	if(CombatComp)
	{
		if(CombatComp->EquippedWeapon)
		{
			DropOrDestroyWeapon(CombatComp->EquippedWeapon);
		}
		if(CombatComp->SecondaryWeapon)
		{
			DropOrDestroyWeapon(CombatComp->SecondaryWeapon);
		}
	}
	MulticastHandleDeath(bPlayerLeftGame);
}

void ABlasterCharacter::MulticastHandleDeath_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	if(BlasterPC)
	{
		BlasterPC->SetHUDWeaponAmmo(0);
		
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(BlasterPC->GetLocalPlayer()))
		{
			if(DefaultMappingContext && InventoryMappingContext)
			{
				Subsystem->RemoveMappingContext(DefaultMappingContext);
				Subsystem->RemoveMappingContext(InventoryMappingContext);
			}
		}
	}
	
	if(CombatComp)
	{
		CombatComp->FireButtonPressed(false);
	}
	
	bDisableGameplay = true;
	bDead = true;
	PlayDeathMontage(IsAiming());

	// Start Dissolve Effect
	CreateDeathDynamicMaterialInstances();
	StartDissolve();

	// Disable Character Movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	
	// Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Spawn Death Bot
	if(DeathBotEffect)
	{
		FVector DeathBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + DeathBotSpawnZOffset);
		DeathBotEffectComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DeathBotEffect, DeathBotSpawnPoint,
			GetActorRotation());

		if(DeathBotSound)
		{
			UGameplayStatics::SpawnSoundAtLocation(GetWorld(), DeathBotSound, GetActorLocation());
		}
	}
	
	if(IsLocallyControlled() && CombatComp && CombatComp->bAiming && CombatComp->EquippedWeapon && CombatComp->EquippedWeapon->GetWeaponType() == EBWeaponType::EWT_Sniper)
	{
		ShowScopeWidget(false);
	}

	if(CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ThisClass::RespawnTimerFinished,
		RespawnDelay);
}

void ABlasterCharacter::DropOrDestroyWeapon(ABWeapon* Weapon)
{
	if(Weapon == nullptr)
	{
		return;
	}
	
	if(Weapon->bDestroyWeaponOnDeath)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}

void ABlasterCharacter::CreateDeathDynamicMaterialInstances()
{
	if(DissolveMaterialInstance_Body1 && DissolveMaterialInstance_Body2 && DissolveMaterialInstance_Head)
	{
		DissolveDynamicMaterialInstance_Body1 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_Body1, this);
		DissolveDynamicMaterialInstance_Body2 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_Body2, this);
		DissolveDynamicMaterialInstance_Head = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_Head, this);

		GetMesh()->SetMaterial(0, DissolveDynamicMaterialInstance_Body1);
		GetMesh()->SetMaterial(1, DissolveDynamicMaterialInstance_Head);
		GetMesh()->SetMaterial(2, DissolveDynamicMaterialInstance_Body2);

		DissolveDynamicMaterialInstance_Body1->SetScalarParameterValue(TEXT("DissolveValue"), -0.55f);
		DissolveDynamicMaterialInstance_Body2->SetScalarParameterValue(TEXT("DissolveValue"), -0.55f);
		DissolveDynamicMaterialInstance_Head->SetScalarParameterValue(TEXT("DissolveValue"), -0.55f);
		
		DissolveDynamicMaterialInstance_Body1->SetScalarParameterValue(TEXT("Glow"), 250.f);
		DissolveDynamicMaterialInstance_Body2->SetScalarParameterValue(TEXT("Glow"), 250.f);
		DissolveDynamicMaterialInstance_Head->SetScalarParameterValue(TEXT("Glow"), 250.f);
	}
}

void ABlasterCharacter::RespawnTimerFinished()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if(BlasterGameMode && !bLeftGame)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}

	if(bLeftGame && IsLocallyControlled())
	{
		OnLeftGameDelegate.Broadcast();
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if(DissolveDynamicMaterialInstance_Body1 && DissolveDynamicMaterialInstance_Body2 && DissolveDynamicMaterialInstance_Head)
	{
		DissolveDynamicMaterialInstance_Body1->SetScalarParameterValue(TEXT("DissolveValue"), DissolveValue);
		DissolveDynamicMaterialInstance_Body2->SetScalarParameterValue(TEXT("DissolveValue"), DissolveValue);
		DissolveDynamicMaterialInstance_Head->SetScalarParameterValue(TEXT("DissolveValue"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrackDelegate.BindDynamic(this, &ThisClass::UpdateDissolveMaterial);
	if(DissolveCurve && DissolveTimelineComp)
	{
		DissolveTimelineComp->AddInterpFloat(DissolveCurve, DissolveTrackDelegate);
		DissolveTimelineComp->Play();
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

void ABlasterCharacter::HideCharacterIfCameraClose()
{
	if(!IsLocallyControlled())
	{
		return;
	}

	// Distance to Character less than CameraThreshold
	if((CameraComponent->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if(CombatComp && CombatComp->EquippedWeapon && CombatComp->EquippedWeapon->GetWeaponMesh())
		{
			CombatComp->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(CombatComp && CombatComp->EquippedWeapon && CombatComp->EquippedWeapon->GetWeaponMesh())
		{
			CombatComp->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABlasterCharacter::ServerEquip_Implementation()
{
	if(CombatComp)
	{
		CombatComp->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::ServerLeaveGame_Implementation()
{
	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	ABPlayerState* BPS = GetPlayerState<ABPlayerState>();
	if(BlasterGameMode && BPS)
	{
		BlasterGameMode->PlayerLeftGame(BPS);
	}
}

void ABlasterCharacter::MulticastGainedTheLead_Implementation()
{
	if(CrownSystem == nullptr)
	{
		return;
	}
	
	if(CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(CrownSystem, GetCapsuleComponent(), FName(),
			GetActorLocation() + FVector(0.f, 0.f, 90.f), GetActorRotation(),
			EAttachLocation::KeepWorldPosition, false);
		CrownComponent->SetNiagaraVariableLinearColor(FString("Color01"), FLinearColor::Red);
	}
	
	if(CrownComponent)
	{
		CrownComponent->Activate();	
	}
}

void ABlasterCharacter::MulticastLostTheLead_Implementation()
{
	if(CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}






