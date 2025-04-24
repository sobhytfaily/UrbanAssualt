// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "shotgun.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API Ashotgun : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);
	virtual void FireShotgun(const TArray<FVector_NetQuantize>& HitTargets);
private:
	UPROPERTY(EditAnywhere)
	uint32 numberOfPellets = 10;

	
};
