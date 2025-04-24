#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Shooter/ShooterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Shooter/PlayerController/ShooterPlayerController.h"
#include "Shooter/ShooterComponent/LagCompensationComponent.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(FireHit.GetActor());
		if (ShooterCharacter && InstigatorController)
		{
			PlayerCharacter = PlayerCharacter == nullptr ? Cast<AShooterCharacter>(GetOwner()) : PlayerCharacter;
			if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
			{
				FireHit.BoneName.ToString() == FString("head") ? PlayerCharacter->ShowHeadShotHitMarker() : PlayerCharacter->ShowHitMarker();
			}
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage)
			{
				const float damageToCause = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;
				UGameplayStatics::ApplyDamage(
					ShooterCharacter,
					damageToCause,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
				
				
			}
			if(!HasAuthority() && bUseServerSideRewind)
			{
				PlayerCharacter = PlayerCharacter == nullptr ? Cast<AShooterCharacter>(OwnerPawn) : PlayerCharacter;
				PlayerController = PlayerController == nullptr ? Cast<AShooterPlayerController>(InstigatorController) : PlayerController;
				if (PlayerCharacter && PlayerController && PlayerCharacter->GetLagCompansationComponent())
				{
					UE_LOG(LogTemp, Warning, TEXT("Hit Rewing"));
					PlayerCharacter->GetLagCompansationComponent()->ServerScoreRequest(
						ShooterCharacter,
						Start,
						HitTarget,
						PlayerController->GetServerTime() - PlayerController->SingleTripTime
					);
				}
			}
			
		}
		if (ImpactParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticle,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
			);
		}
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint
			);
		}

		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			OutHit.ImpactPoint = End;
		}
		if (BeamParticle)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticle,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}