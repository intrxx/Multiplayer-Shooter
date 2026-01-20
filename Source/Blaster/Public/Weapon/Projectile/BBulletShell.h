// Copyright 2026 out of sCope team - intrxx

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BBulletShell.generated.h"

class USoundCue;
class UStaticMeshComponent;

UCLASS()
class BLASTER_API ABBulletShell : public AActor
{
	GENERATED_BODY()
	
public:	
	ABBulletShell();

protected:
	virtual void BeginPlay() override;

	void BulletShellDestroy();
	
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(VisibleAnywhere, Category = "Blaster|BulletShell")
	TObjectPtr<UStaticMeshComponent> ShellMeshComp;

	UPROPERTY(EditAnywhere, Category = "Blaster|BulletShell")
	float ShellEjectionImpulse = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Blaster|BulletShell")
	TObjectPtr<USoundCue> ShellDropSound;

	// Time until Bullet Shell is destroyed after it hits the ground
	UPROPERTY(EditAnywhere, Category = "Blaster|BulletShell")
	float TimeUntilDestroyed = 4.f;

	FTimerDelegate ShellDropTimerDelegate;
	FTimerHandle ShellDropTimerHandle;
};
