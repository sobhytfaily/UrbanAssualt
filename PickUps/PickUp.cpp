// Fill out your copyright notice in the Description page of Project Settings.


#include "PickUp.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Shooter/Weapon/WeaponTypes.h"
#include "Components/SphereComponent.h"
APickUp::APickUp()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("overlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetSphereRadius(150.f);
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	PickUpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickUp Mesh"));
	PickUpMesh->SetupAttachment(OverlapSphere);
	PickUpMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickUpMesh->SetRenderCustomDepth(true);
	PickUpMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);
}

void APickUp::Destroyed()
{
	Super::Destroyed();
	if (PickUpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickUpSound, GetActorLocation());
	}
}

void APickUp::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(BindOverlapTimer, this, &APickUp::BindOverlapTimerFinished, BindOverlapTime);
	}
}

void APickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void APickUp::BindOverlapTimerFinished()
{

	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickUp::OnSphereOverlap);
}

