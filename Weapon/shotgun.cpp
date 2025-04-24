// Fill out your copyright notice in the Description page of Project Settings.


#include "shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Shooter/ShooterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Shooter/PlayerController/ShooterPlayerController.h"
#include "Shooter/ShooterComponent/LagCompensationComponent.h"
#include "Particles/ParticleSystemComponent.h"



void Ashotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (MuzzleFlashSocket == nullptr) return;

    const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
    const FVector TraceStart = SocketTransform.GetLocation();
    const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
    const FVector SphereCenter = TraceStart + ToTargetNormalized * distanceToSphere;

    for (uint32 i = 0; i < numberOfPellets; i++)
    {
        const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, sphereRadius);
        const FVector EndLoc = SphereCenter + RandVec;
        FVector ToEndLoc = EndLoc - TraceStart;
        ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGHT / ToEndLoc.Size();
        HitTargets.Add(ToEndLoc);
    }
}

void Ashotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
    AWeapon::Fire(FVector());
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn == nullptr) return;
    AController* InstigatorController = OwnerPawn->GetController();
    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (MuzzleFlashSocket)
    {
        const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
        const FVector Start = SocketTransform.GetLocation();
        TMap<AShooterCharacter*, uint32> HitMap; //Maps hit character to number of hits
        TMap<AShooterCharacter*, uint32> HeadShotHitMap; //Maps hit character to number of headshots

        for (auto HitTarget : HitTargets)
        {
            FHitResult FireHit;
            WeaponTraceHit(Start, HitTarget, FireHit);
            AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(FireHit.GetActor());
            if (ShooterCharacter)
            {
                const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");
                if (bHeadShot)
                {
                    if (HeadShotHitMap.Contains(ShooterCharacter)) HeadShotHitMap[ShooterCharacter]++;
                    else HeadShotHitMap.Emplace(ShooterCharacter, 1);
                }
                else
                {
                    if (HitMap.Contains(ShooterCharacter)) HitMap[ShooterCharacter]++;
                    else HitMap.Emplace(ShooterCharacter, 1);
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
                        FireHit.ImpactPoint,
                        .5f,
                        FMath::FRandRange(-.5f, .5f)
                    );
                }
            }

        }
        TArray<AShooterCharacter*> HitCharacters;
        //maps character hit to total damage
        TMap<AShooterCharacter*, float> DamageMap;
        //Calculates body damage to each character hit
        for (auto& Hitpair : HitMap)
        {
            if (Hitpair.Key)
            {
                DamageMap.Emplace(Hitpair.Key, Hitpair.Value * Damage);

                HitCharacters.AddUnique(Hitpair.Key);
                if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
                {
                    PlayerCharacter->ShowHitMarker();
                }
            }
        }
        //Calculates headshot damage to each character hit
        for (auto& HeadShotHitpair : HeadShotHitMap)
        {
            if (HeadShotHitpair.Key)
            {
                if (DamageMap.Contains(HeadShotHitpair.Key)) DamageMap[HeadShotHitpair.Key] += HeadShotHitpair.Value * HeadShotDamage;
                else DamageMap.Emplace(HeadShotHitpair.Key, HeadShotHitpair.Value * HeadShotDamage);

                HitCharacters.AddUnique(HeadShotHitpair.Key);
                if (PlayerCharacter && PlayerCharacter->IsLocallyControlled())
                {
                    PlayerCharacter->ShowHeadShotHitMarker();
                }
            }
        }
        PlayerCharacter = PlayerCharacter == nullptr ? Cast<AShooterCharacter>(GetOwner()) : PlayerCharacter;
        //loop through damage map to calculate total damage for each character
        for (auto& DamagePair : DamageMap)
        {
            if (DamagePair.Key && InstigatorController)
            {
                bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
                if (HasAuthority() && bCauseAuthDamage)
                {
                    // Applies damage to each character seperatly 
                    UGameplayStatics::ApplyDamage(
                        DamagePair.Key,
                        DamagePair.Value,
                        InstigatorController,
                        this,
                        UDamageType::StaticClass()
                    );
                }
            }
        }
        if (!HasAuthority() && bUseServerSideRewind )
        {
            PlayerCharacter = PlayerCharacter == nullptr ? Cast<AShooterCharacter>(OwnerPawn) : PlayerCharacter;
            PlayerController = PlayerController == nullptr ? Cast<AShooterPlayerController>(InstigatorController) : PlayerController;
            if (PlayerCharacter && PlayerController && PlayerCharacter->GetLagCompansationComponent() && PlayerCharacter->IsLocallyControlled() && PlayerCharacter->IsLocallyControlled())
            {
                PlayerCharacter->GetLagCompansationComponent()->ShotgunServerScoreRequest(
                    HitCharacters,
                    Start,
                    HitTargets,
                    PlayerController->GetServerTime() - PlayerController->SingleTripTime
                );
            }
        }
    }
}


