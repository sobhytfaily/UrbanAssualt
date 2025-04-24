// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon.h"
#include "Shooter/CombatState.h"
void UShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}
void UShooterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	if (ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	if (ShooterCharacter == nullptr) return;
		FVector Velocity = ShooterCharacter->GetVelocity();
		Velocity.Z = 0.f;
		speed = Velocity.Size();
		IsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();
		isAccerlating = ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
		bWeaponEquipped = ShooterCharacter->IsWeaponEquipped();
		EquippedWeapon = ShooterCharacter->GetEqiuppedWeapon();
		bIsCrouched = ShooterCharacter->bIsCrouched;
		bAiming = ShooterCharacter->IsAiming();
		TurningInPlace = ShooterCharacter->GetTurningInPlace();
		bRotateRootBone = ShooterCharacter->ShouldRotateRootBone();
		bElimed = ShooterCharacter->IsElimed();
		bHoldingTheFlag = ShooterCharacter->IsHoldingTheFlag();
		//Offset Yaw for Strafing
		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
		FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
		DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 15.f);
		YawOffset = DeltaRotation.Yaw;
		//Offset Lean
		CharacterRotationLastFrame = CharacterRotation;
		CharacterRotation = ShooterCharacter->GetActorRotation();
		const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
		const float target = Delta.Yaw / DeltaTime;
		const float Interp = FMath::FInterpTo(Lean, target, DeltaTime, 6.f);
		Lean = FMath::Clamp(Interp, -90.f, 90.f);
		AO_Yaw = ShooterCharacter->GetAO_Yaw();
		AO_Pitch = ShooterCharacter->GetAO_Pitch();


		if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && ShooterCharacter->GetMesh())
		{
			LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
			FVector OutPosition;
			FRotator OutRotation;
			ShooterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
			LeftHandTransform.SetLocation(OutPosition);
			LeftHandTransform.SetRotation(FQuat(OutRotation));
			if (ShooterCharacter->IsLocallyControlled())
			{
				bLocallyControlled = true;
				FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
				FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - ShooterCharacter->GetHitTarget()));
				RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 100.f);
			}
		}
		

		bUseFabrick = ShooterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
		bool bFabrickOverride =
			ShooterCharacter->IsLocallyControlled() &&
			ShooterCharacter->GetCombatState() != ECombatState::ECS_ThrowingGernade &&
			ShooterCharacter->bFinishedSwaping;
		if (bFabrickOverride)
		{
			bUseFabrick = !ShooterCharacter->IsLocallyReloading();
		}
		bUseAimOffsets = ShooterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !ShooterCharacter->getDisabledGameplay();
		bTransformRightHand = ShooterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !ShooterCharacter->getDisabledGameplay();
}

