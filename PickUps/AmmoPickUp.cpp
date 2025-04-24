// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickUp.h"
#include "Shooter/ShooterCharacter.h"
#include "Shooter/ShooterComponent/CombatComponent.h"

void AAmmoPickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter)
	{
		UCombatComponent* ShooterCombat = ShooterCharacter->getCombatComponent();
		if (ShooterCombat)
		{
			ShooterCombat->PickUpAmmo(weaponType, AmmoAmount);
		}
	}
	Destroy();
}
