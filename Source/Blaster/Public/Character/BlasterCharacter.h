// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

public:	
	

private:
	UPROPERTY(VisibleAnywhere, Category = "Blaster|Camera")
	USpringArmComponent* SpringArmComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "Blaster|Camera")
	UCameraComponent* CameraComponent;
	
};
