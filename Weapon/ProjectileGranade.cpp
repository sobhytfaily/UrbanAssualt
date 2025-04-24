// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGranade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileGranade::AProjectileGranade()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Granade mesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true;
}


void AProjectileGranade::BeginPlay()
{
	AActor::BeginPlay();

	StartDestroyTimer();
	SpawnTrailSystem();

	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGranade::OnBounce);

}

void AProjectileGranade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	if(BounceSound)
	UGameplayStatics::SpawnSoundAtLocation(
		this,
		BounceSound,
		GetActorLocation(),
		FRotator::ZeroRotator
	);
}
void AProjectileGranade::Destroyed()
{
	ExplodeDamage();
	Super::Destroyed();

}