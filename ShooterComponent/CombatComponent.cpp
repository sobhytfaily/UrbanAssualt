// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Shooter/Weapon.h"
#include "Shooter/ShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Shooter/PlayerController/ShooterPlayerController.h"
#include "Shooter/ShooterHUD.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Shooter/ShooterAnimInstance.h"
#include "Sound/SoundCue.h"
#include "Shooter/Weapon/Projectile.h"
#include "Shooter/Weapon/Shotgun.h"


UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
		
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Gerandes);
	DOREPLIFETIME(UCombatComponent, bHoldingTheFlag);

}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Controller)
	{
		Character = Character == nullptr ? Cast<AShooterCharacter>(Controller->GetCharacter()) : Character;
	}

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if (Character->GetFollowCamera())
		{
			DefualtFov  = 120.f;
			CurrentFov = DefualtFov;
		}
	}
	if (Character->HasAuthority())
	{
		IntializeCarriedAmmo();
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;
		SetHudCrosshairs(DeltaTime);
		InterpFov(DeltaTime);
	}
	ReloadEmptyWeapon();
}

void UCombatComponent::SetHudCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<AShooterHud>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}
			//Calculate Crosshair Spread
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread =
				0.5f +
				CrosshairVelocityFactor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor;

			HUD->setHUDPackage(HUDPackage);
		}
	}
}
void UCombatComponent::InterpFov(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;
	if (bAiming)
	{
		if (!Character->inFirst)
		{
			CurrentFov = FMath::FInterpTo(CurrentFov, EquippedWeapon->GetZoomedFov(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
		}
		if (Character->inFirst && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
		{
			CurrentFov = FMath::FInterpTo(CurrentFov, EquippedWeapon->GetZoomedFov(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
		}
	}
	else
	{

		CurrentFov = FMath::FInterpTo(CurrentFov, DefualtFov, DeltaTime, ZoomInterpSpeed);

		
	}
	
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFov);
	}
}
void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;

	bCanFire = true;
	if (bFireButtonPressed)
	{
		if (EquippedWeapon->bAutomatic)
		{
			Fire();

		}
	}
	ReloadEmptyWeapon();
}
void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr ) return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}
void UCombatComponent::OnRep_EuippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		PlayEquipedWeaponSound(EquippedWeapon);
		EquippedWeapon->SetHudAmmo();
	}
}
void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToBackPack(SecondaryWeapon);
		PlayEquipedWeaponSound(SecondaryWeapon);
	}
}
void UCombatComponent::Fire()
{
	
	if(CanFire())
	{
		bCanFire = false;
		if (EquippedWeapon)
		{
			CrosshairShootingFactor = .75f;
			switch (EquippedWeapon->FireType)
			{
			case EFireType::EFT_Projectile:
				FireProjectileWeapon();
				break;
			case EFireType::EFT_HitScan:
				FireHitScanWeapon();
				break;
			case EFireType::EFT_Shotgun:
				FireShotgun();
				break;
			default:
				break;
			}
		}
		StartFireTimer();
	}
}
void UCombatComponent::FireProjectileWeapon()
{
	if (EquippedWeapon)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if(Character && !Character->HasAuthority())	LocalFire(HitTarget);
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}
	
}
void UCombatComponent::FireHitScanWeapon()
{
	if (EquippedWeapon)
	{
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if (Character && !Character->HasAuthority())	LocalFire(HitTarget);
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}
}
void UCombatComponent::FireShotgun()
{
	Ashotgun* Shotgun = Cast<Ashotgun>(EquippedWeapon);
	if (Shotgun)
	{
		TArray<FVector_NetQuantize> HitTargets;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
		if (Character && !Character->HasAuthority())	LocalShotgunFire(HitTargets);
		ServerShotgunFire(HitTargets, EquippedWeapon->FireDelay);
	}
}
void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if(bFireButtonPressed)
	{
		
		Fire();
	}
}
void UCombatComponent::ShotgunShellReload()
{
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}
void UCombatComponent::JumpToShotgunEnd()
{
	if (Character == nullptr)  return;
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		if (Character->GetReloadMontage())
		{
			AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
		}
	}
}
void UCombatComponent::PickUpAmmo(EWeaponType weaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(weaponType))
	{
		CarriedAmmoMap[weaponType] = FMath::Clamp(CarriedAmmoMap[weaponType] + AmmoAmount, 0, MaxCarriedAmmo);
		UpdateCarriedAmmo();
	}
	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == weaponType)
	{
		Reload();
	}
}
void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGHT;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
		if (!TraceHitResult.bBlockingHit) TraceHitResult.ImpactPoint = End;
	}
	

}
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	MulticastFire(TraceHitTarget);
}
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFire(TraceHitTarget);
}
void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	MulticastShotgunFire(TraceHitTargets);
}
void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalShotgunFire(TraceHitTargets);
}
void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{	
	if (EquippedWeapon == nullptr) return;
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		if (EquippedWeapon->GetWeaponType() == EWeaponType::EWT_AssualtRifle && HUDPackage.CrosshairColor == FColor::Red)
		{
			Character->ShowHitMarker();
		}
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	Ashotgun* Shotgun = Cast<Ashotgun>(EquippedWeapon);
	if (Shotgun == nullptr || Character == nullptr) return;
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		bLocallyReloading = false;
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

void UCombatComponent::SeverSetAiming_Implementation(bool bisAiming)
{
	bAiming = bisAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bisAiming ? AimWalkSpeed : BaseWalkSpeed;
	}

}
void UCombatComponent::SetAiming(bool bisAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	bAiming = bisAiming;
	SeverSetAiming(bisAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bisAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSnipeScopeWidget(bisAiming);
	}
	if (Character->IsLocallyControlled())
	{
		bAimButtonPressed = bisAiming;
	}
}
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	
	if (WeaponToEquip->GetWeaponType() == EWeaponType::EWT_Flag)
	{
		Character->Crouch();
		bHoldingTheFlag = true;
		WeaponToEquip->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachFlagToLeftHand(WeaponToEquip);
		WeaponToEquip->SetOwner(Character);
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
		Character->bUseControllerRotationYaw = false;

	}
	else
	{
		if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
		{
			EquipSecondayWeapon(WeaponToEquip);
		}
		else
		{
			EquipPrimaryWeapon(WeaponToEquip);
			UpdateCarriedAmmo();
			UpdateAmmoValues();
		}

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}

}
void UCombatComponent::SwapWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied || Character == nullptr) return;
	
	Character->PlaySwapMontage();
	Character->bFinishedSwaping = false;
	CombatState = ECombatState::ECS_SwapingWeapons;

	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;
}
void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr || Character == nullptr) return;
	DropEquippedWeapon();
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHudAmmo();
	UpdateCarriedAmmo();
	PlayEquipedWeaponSound(EquippedWeapon);
	ReloadEmptyWeapon();
	EquippedWeapon->EnableCustomDepth(false);
}
void UCombatComponent::EquipSecondayWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr || Character == nullptr) return;
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackPack(WeaponToEquip);
	SecondaryWeapon->SetOwner(Character);
	PlayEquipedWeaponSound(SecondaryWeapon);
	SecondaryWeapon->SetOwner(Character);
}
void UCombatComponent::OnRep_Aiming()
{
	if (Character && Character->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
	}
}
void UCombatComponent::ReloadEmptyWeapon()
{

	if (EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}
void UCombatComponent::PlayEquipedWeaponSound(AWeapon* WeaponToEquip)
{
	
	if (Character && WeaponToEquip && WeaponToEquip->EquipeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponToEquip->EquipeSound, Character->GetActorLocation());
	}
}
void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller  == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHudCarriedAmmo(CarriedAmmo);
	}
}
void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || ActorToAttach == nullptr || Character->GetMesh() == nullptr) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}
void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || ActorToAttach == nullptr || Character->GetMesh() == nullptr || EquippedWeapon == nullptr) return;
	bool bUsePistolSocket = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol || EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SMG;
	FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");
	const USkeletalMeshSocket* HandSocket =  Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}
void UCombatComponent::AttachActorToBackPack(AActor* ActorToAttach)
{
	if (Character == nullptr || ActorToAttach == nullptr || Character->GetMesh() == nullptr) return;
	const USkeletalMeshSocket* BackPackSocket = Character->GetMesh()->GetSocketByName(FName("BackSocket"));
	if (BackPackSocket)
	{
		BackPackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}

}
void UCombatComponent::AttachFlagToLeftHand(AWeapon* Flag)
{
	if (Character == nullptr || Flag == nullptr || Character->GetMesh() == nullptr) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("FlagSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(Flag, Character->GetMesh());
	}
}
void UCombatComponent::DropEquippedWeapon()	
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}
void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull() && !bLocallyReloading)
	{
		ServerReload();
		HandleReload();
		bLocallyReloading = true;
	}
}
void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;


	CombatState = ECombatState::ECS_Reloading;
	if (!Character->IsLocallyControlled())
	{
		HandleReload();
	}
}
void UCombatComponent::HandleReload()
{
	if (Character)
	{
		Character->PlayReloadMontage();
	}
}
int32 UCombatComponent::AmountToReload() 
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmmountCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}
void UCombatComponent::ThrowGernade()
{
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr || Gerandes == 0) return;

	CombatState = ECombatState::ECS_ThrowingGernade;
	if (Character)
	{
		Character->PlayGernadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGernade(true);
	}
	if (Character && !Character->HasAuthority())
	{
		ServerThrowGernade();
	}
	if (Character && Character->HasAuthority())
	{
		Gerandes = FMath::Clamp(Gerandes - 1, 0, MaxGerandes);
		UpdateHudGerandes();
	}

}

void UCombatComponent::ServerThrowGernade_Implementation()
{
	if (Gerandes == 0) return;
	CombatState = ECombatState::ECS_ThrowingGernade;
	if (Character)
	{
		Character->PlayGernadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGernade(true);
	}
	Gerandes = FMath::Clamp(Gerandes - 1, 0, MaxGerandes);
	UpdateHudGerandes();
}
void UCombatComponent::UpdateHudGerandes()
{
	Controller = Controller ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHudGernades(Gerandes);
	}
}
void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break; 
	case ECombatState::ECS_Reloading:
		if (Character && !Character->IsLocallyControlled())
		{
			HandleReload();
		}
		break;
		case ECombatState::ECS_ThrowingGernade:
			if (Character && !Character->IsLocallyControlled())
			{
				Character->PlayGernadeMontage();
				AttachActorToLeftHand(EquippedWeapon);	
				ShowAttachedGernade(true);
			}
		break;
		case ECombatState::ECS_SwapingWeapons:
			if (Character && !Character->IsLocallyControlled())
			{
				Character->PlaySwapMontage();
			}
			break;
	default:
		break;
	}
}
void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHudCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(ReloadAmount);
}
void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{

		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHudCarriedAmmo(CarriedAmmo);
	}

	EquippedWeapon->AddAmmo(1);
	bCanFire = true;
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}
void UCombatComponent::ShowAttachedGernade(bool bShowGerande)
{
	if (Character && Character->GetAttachedGernade())
	{
		Character->GetAttachedGernade()->SetVisibility(bShowGerande);
	}
}
void UCombatComponent::OnRep_Gernades()
{
	UpdateHudGerandes();
}
void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;
	bLocallyReloading = false;
	if(Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if(bFireButtonPressed)
	{
		Fire();
	}
}
void UCombatComponent::FinishSwap()
{
	if (Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
	}
	if (Character)
	{
		Character->bFinishedSwaping = true;
	}
}
void UCombatComponent::FinishSwapWeapons()
{
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);

	EquippedWeapon->SetHudAmmo();
	UpdateCarriedAmmo();
	PlayEquipedWeaponSound(EquippedWeapon);

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackPack(SecondaryWeapon);
	
}
void UCombatComponent::ThrowGernadeFinish()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}
void UCombatComponent::LunchGernade()
{
	ShowAttachedGernade(false);
	if(Character && Character->IsLocallyControlled())
	ServerLunchGernade(HitTarget);
}
void UCombatComponent::OnRep_HoldingTheFlag()
{
	if (bHoldingTheFlag && Character && Character->IsLocallyControlled())
	{
		Character->Crouch();
	}
}
bool UCombatComponent::ShouldSwapWeapons()
{
	return EquippedWeapon != nullptr && SecondaryWeapon != nullptr;
}
void UCombatComponent::ServerLunchGernade_Implementation(const FVector_NetQuantize& Target)
{
	if (Character && GernadeClass && Character->GetAttachedGernade())
	{
		const FVector StartingLocation = Character->GetAttachedGernade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(GernadeClass, StartingLocation + FVector(0.f, 0.f, 20.f), ToTarget.Rotation(), SpawnParams);
		}
	}
}
bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	if (!EquippedWeapon->IsEmpty() &&
		bCanFire &&
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun
		)
	{
		return true;
	}
	if (bLocallyReloading) return false;
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}
void UCombatComponent::OnRep_CarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHudCarriedAmmo(CarriedAmmo);
	}
	bool bJumpToShotgunEnd =
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_ShotGun &&
		CarriedAmmo == 0;
	if (bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}
void UCombatComponent::IntializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssualtRifle, StartingAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLuncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SMG, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_ShotGun, StartingShotGunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperRifleAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GranadeLuncher, StartingGranadeLuncherAmmo);
}
