// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AFlag : public AWeapon
{
	GENERATED_BODY()
public:
	AFlag();
	virtual void Dropped() override;
	void ResetFlag();
protected:
	virtual void OnWeaponEquipped() override;
	virtual void OnWeaponDropped() override;
	virtual void BeginPlay() override;
private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagMesh;
	FTransform IntialTransform;


public:
	FORCEINLINE FTransform GetIntialTransform() const { return IntialTransform; }
};
