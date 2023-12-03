// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/Public/Character/BlasterCharacter.h"

#include "Physics/BlasterCollisionChannels.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Blaster/BlasterGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "BlasterComponents/BCombatComponent.h"
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
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Player/BPlayerState.h"
#include "BlasterTypes/BWeaponTypes.h"

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
	
	GetMesh()->SetCollisionObjectType(ECC_ObjectChannel_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadComponent"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComp = CreateDefaultSubobject<UBCombatComponent>(TEXT("CombatComponen"));
	CombatComp->SetIsReplicated(true);
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	TurningInPlace = EBTurningInPlace::ETIP_NotTurning;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 750.f, 0.f);

	DissolveTimelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComp"));

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
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

	UpdateHUDHealth();
	
	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}
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
	return  Velocity.Size();
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
			AnimInstance->Montage_JumpToSection(TEXT("Rifle")); // Don't have EWT_RocketLauncher Reload Anim right now
			break;
		case EBWeaponType::EWT_Pistol:
			AnimInstance->Montage_JumpToSection(TEXT("Rifle")); // Don't have EWT_Pistol Reload Anim right now
			break;
		case EBWeaponType::EWT_SubMachineGun:
			AnimInstance->Montage_JumpToSection(TEXT("Rifle")); // Don't have EWT_SubMachineGun Reload Anim right now
			break;
		case EBWeaponType::EWT_Shotgun:
			AnimInstance->Montage_JumpToSection(TEXT("Rifle")); // Don't have EWT_Shotgun Reload Anim right now
			break;
		default:
			break;
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
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	
	UpdateHUDHealth();
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

void ABlasterCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPC =  BlasterPC == nullptr ? Cast<ABPlayerController>(Controller) : BlasterPC;
	if(BlasterPC)
	{
		BlasterPC->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::HandleDeath()
{
	MulticastHandleDeath();

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ThisClass::RespawnTimerFinished, RespawnDelay);
}

void ABlasterCharacter::MulticastHandleDeath_Implementation()
{
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

	if(CombatComp && CombatComp->EquippedWeapon)
	{
		CombatComp->EquippedWeapon->Dropped();
	}

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
	if(BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
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






