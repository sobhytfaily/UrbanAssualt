// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Shooter/ShooterCharacter.h"
#include "Shooter/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"
void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket =  GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* world = GetWorld();
	if (MuzzleFlashSocket && world)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		//From muzzleFlash socket to hit location crosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnedProjectile = nullptr;
		if (bUseServerSideRewind)
		{
			if (InstigatorPawn->HasAuthority()) //server
			{
				if (InstigatorPawn->IsLocallyControlled()) //server host - user replicated projectile
				{
					SpawnedProjectile = world->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
					SpawnedProjectile->damage = Damage;
					SpawnedProjectile->headShotDamage = HeadShotDamage;
				}
				else // server, not locally controlled spawn non replicated projectile, no SSR
				{
					SpawnedProjectile = world->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
				}	
			}
			else // client, Using SSR
			{
				if (InstigatorPawn->IsLocallyControlled()) // Client Locally Controller Spawn non replicated projectile, use SSR
				{
					SpawnedProjectile = world->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->IntialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->IntialSpeed;
				}
				else // Client, Not locally controller - spawn non replicated projectile, no SSR
				{
					SpawnedProjectile = world->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		}
		else // Not Using SSR 
		{
			if (InstigatorPawn->HasAuthority())
			{
				SpawnedProjectile = world->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->damage = Damage;	
				SpawnedProjectile->headShotDamage = HeadShotDamage;
				SpawnedProjectile->Owner = InstigatorPawn;
			}
		}
	}
}
