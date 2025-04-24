// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Shooter/ShooterCharacter.h"
#include "Shooter/Weapon.h"
#include "Shooter/PlayerController/ShooterPlayerController.h"
#include "Shooter/ShooterComponent/LagCompensationComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = IntialSpeed;
	ProjectileMovementComponent->MaxSpeed = IntialSpeed;
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& event)
{
	Super::PostEditChangeProperty(event);

	FName PropertyName = event.Property != nullptr ? event.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, IntialSpeed))
	{
		if (ProjectileMovementComponent)
		{
			ProjectileMovementComponent->InitialSpeed = IntialSpeed;
			ProjectileMovementComponent->MaxSpeed = IntialSpeed;
		}
	}
}
#endif
void AProjectileBullet::OnHit(UPrimitiveComponent *HitCom, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse, const FHitResult &Hit)
{
	AShooterCharacter* OwnerCharacter = Cast<AShooterCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		AShooterPlayerController* OwnerContoller = Cast<AShooterPlayerController>(OwnerCharacter->Controller);
		if (OwnerContoller)
		{
			if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
			{
				const float damageToCause = Hit.BoneName.ToString() == FString("head") ? headShotDamage : damage;
				UGameplayStatics::ApplyDamage(OtherActor, damageToCause, OwnerContoller, this, UDamageType::StaticClass());
				Super::OnHit(HitCom, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}
			AShooterCharacter* HitCharacter = Cast<AShooterCharacter>(OtherActor);
			if (bUseServerSideRewind && OwnerCharacter->GetLagCompansationComponent() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				OwnerCharacter->GetLagCompansationComponent()->ProjectileServerScoreRequest(HitCharacter, TraceStart, IntialVelocity, OwnerContoller->GetServerTime() - OwnerContoller->SingleTripTime);
			}
		}
	}
	Super::OnHit(HitCom, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	//FPredictProjectilePathParams PathParams;
	//PathParams.bTraceWithChannel = true;
	//PathParams.bTraceWithCollision = true;
	//PathParams.DrawDebugTime = 5.f;
	//PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	//PathParams.LaunchVelocity = GetActorForwardVector() * IntialSpeed;
	//PathParams.MaxSimTime = 4.f;
	//PathParams.ProjectileRadius = 5.f;
	//PathParams.SimFrequency = 30.f;
	//PathParams.StartLocation = GetActorLocation();
	//PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;
	//PathParams.ActorsToIgnore.Add(this);
	//FPredictProjectilePathResult PathResult;
	//UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
}
