// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/BBulletShell.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ABBulletShell::ABBulletShell()
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	ShellMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletShellMesh"));
	SetRootComponent(ShellMeshComp);

	ShellMeshComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	ShellMeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	ShellMeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	ShellMeshComp->SetSimulatePhysics(true);
	ShellMeshComp->SetNotifyRigidBodyCollision(true);

	ShellEjectionImpulse = 3.f;
	TimeUntilDestroyed = 4.f;

	ShellDropTimerDelegate.BindUObject(this, &ThisClass::BulletShellDestroy);
}

void ABBulletShell::BeginPlay()
{
	Super::BeginPlay();

	ShellMeshComp->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
	ShellMeshComp->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
}

void ABBulletShell::BulletShellDestroy()
{
	GetWorldTimerManager().ClearTimer(ShellDropTimerHandle);
	Destroy();
}

void ABBulletShell::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                          FVector NormalImpulse, const FHitResult& Hit)
{
	if(ShellDropSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ShellDropSound, GetActorLocation());
	}
	ShellMeshComp->SetNotifyRigidBodyCollision(false);
	
	GetWorldTimerManager().SetTimer(ShellDropTimerHandle, ShellDropTimerDelegate, TimeUntilDestroyed,
		false);
}


