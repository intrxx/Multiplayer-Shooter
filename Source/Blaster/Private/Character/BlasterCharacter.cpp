// Fill out your copyright notice in the Description page of Project Settings.


#include "Blaster/Public/Character/BlasterCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "Blaster/BlasterGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Input/BlasterEnhancedInputComponent.h"
#include "Player/BPlayerController.h"

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

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadComponent"));
	OverheadWidget->SetupAttachment(RootComponent);
	
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Hiding the Revenant weapon which is attached to the mesh
	GetMesh()->HideBoneByName(TEXT("weapon_l"), PBO_None);

	if (ABPlayerController* PC = Cast<ABPlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
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
		&ACharacter::Jump);

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

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}



