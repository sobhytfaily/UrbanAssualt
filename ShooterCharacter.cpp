// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapon.h"
#include "ShooterAnimInstance.h"
#include "Shooter/GameState/ShooterGameState.h"
#include "Shooter/PlayerStart/TeamPlayerStart.h"
#include "Shooter/ShooterComponent/CombatComponent.h"
#include "Shooter/ShooterComponent/BuffComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Shooter/Shooter.h"
#include "Shooter/PlayerController/ShooterPlayerController.h"
#include "ShooterGameMode.h"
#include "TimerManager.h"
#include "Shooter/PlayerState/ShooterPlayerState.h"
#include "Shooter/Weapon/WeaponTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/boxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Shooter/ShooterComponent/LagCompensationComponent.h"

AShooterCharacter::AShooterCharacter()
{

	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->bUsePawnControlRotation = true;
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	//OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	//OverheadWidget->SetupAttachment(RootComponent); 

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
	
	DesolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DesolveTimelineComponent"));
	AttachedGernade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GernadeMesh"));
	AttachedGernade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGernade->SetupAttachment(GetMesh(), FName("RightHandSocket"));
	MaskMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MaskMesh"));
	MaskMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MaskMesh->SetupAttachment(GetMesh(), FName("headSocket"));
	MaskMesh->SetVisibility(true);

	//HitBoxies for server side rewind

	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxies.Add(FName("head"), head);


	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxies.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxies.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxies.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxies.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxies.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxies.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxies.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxies.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxies.Add(FName("hand_r"), hand_r);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxies.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxies.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxies.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxies.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxies.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxies.Add(FName("foot_r"), foot_r);

	for (auto Box : HitCollisionBoxies)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}


void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(AShooterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AShooterCharacter, Health);
	DOREPLIFETIME(AShooterCharacter, bDisableGameplay);
}
void AShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{ 
		Combat->Character = this;
	}
	if (Buff)
	{
		Buff->Character = this;	
	}
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<AShooterPlayerController>(Controller);
		}
	}
	
}

void AShooterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	UpdateHudHealth();
	GetMesh()->UnHideBoneByName(FName("neck_01"));
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AShooterCharacter::RecieveDamage);
	}
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 100.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint
		);
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSoundSpawn,
			GetActorLocation()
		);
	}
	if (AttachedGernade)
	{
		AttachedGernade->SetVisibility(false);
	}
	SpawnDefaultWeapon();
	UpdateHudAmmo();
	ChangeFOV();
	if (FollowCamera)
	{
		FollowCamera->FieldOfView = 120.f;
	}
	IntializedHud = false;
}
void AShooterCharacter::UpdateHudHealth()
{
	PlayerController = PlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHudHealth(Health, MaxHealth);
	}
}
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Combat && Combat->EquippedWeapon && !IntializedHud)
	{
		Combat->EquippedWeapon->ShowPickUpWidget(false);
		Combat->UpdateCarriedAmmo();
		Combat->UpdateHudGerandes();
		IntializedHud = true;
	}
	UpdateHudAmmo();
	if (!Combat)
	{
		Combat = Cast<UCombatComponent>(getCombatComponent());
		Combat->Character = this;
	}
	RotateInPlace(DeltaTime);
	DeltaTimeFov = DeltaTime;
	if (inFirst)
	{
		if (bIsCrouched)
		{
			CameraBoom->SocketOffset.Z = FMath::FInterpTo(CameraBoom->SocketOffset.Z, -20.f, DeltaTime, 20.f); 
		}
		else
		{
			CameraBoom->SocketOffset.Z = FMath::FInterpTo(CameraBoom->SocketOffset.Z, 10.f, DeltaTime, 20.f);
		}
	}
	HideCameraWhenCharacterClose();

	PollInit();
	if (Changefov)
	{
		if (!inFirst)
		{
			if (CameraBoom && FollowCamera && GetMesh())
			{

				CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, FVector(30, 0, 0), DeltaTime, 10.f);
				CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, 0.f, DeltaTime, 10.f);
				if (CameraBoom->SocketOffset == FVector(30, 0, 0) && CameraBoom->TargetArmLength == 0.f)
				{
					inFirst = true;
					EPhysBodyOp body = EPhysBodyOp::PBO_None;
					if (IsLocallyControlled())
					{
						GetMesh()->HideBoneByName(FName("neck_01"), body);
						MaskMesh->SetVisibility(false);
					}
					Changefov = false;

				}

			}

		}
		else
		{
			CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, FVector(0, 100, 50), DeltaTime, 20.f);
			CameraBoom->TargetArmLength = FMath::FInterpTo(CameraBoom->TargetArmLength, 300.f, DeltaTime, 10.f);
			GetMesh()->UnHideBoneByName(FName("neck_01"));
			MaskMesh->SetVisibility(true);
			if (CameraBoom->SocketOffset.Y >= 99)
			{
				Changefov = false;
				inFirst = false;
			}
		}
	}
	
}
void AShooterCharacter::RotateInPlace(float DeltaTime)
{
	if (Combat && Combat->bHoldingTheFlag)
	{
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (Combat && Combat->EquippedWeapon)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;
	}
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateYO_Ptich();
	}
}
void AShooterCharacter::UpdateHudAmmo()
{
	PlayerController = PlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : PlayerController;
	if (PlayerController && Combat && Combat->EquippedWeapon)
	{
		PlayerController->SetHudCarriedAmmo(Combat->CarriedAmmo);
		PlayerController->SetHudWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}

}
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AShooterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AShooterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AShooterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("Gernade", IE_Pressed, this, &AShooterCharacter::GernadeButtonPressed);
	PlayerInputComponent->BindAction("ChangeFOV", IE_Pressed, this, &AShooterCharacter::ChangeFOV);

}
void AShooterCharacter::MoveForward(float Value)
{
	if (bDisableGameplay)
	{
		return;
	}
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotator).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}
void AShooterCharacter::MoveRight(float Value)
{
	if (bDisableGameplay)
	{
		return;
	}
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotator).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}
void AShooterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}
void AShooterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}
void AShooterCharacter::EquipButtonPressed()
{
	if (bDisableGameplay)
	{
		return;
	}
	if (Combat)
	{
		if (Combat->bHoldingTheFlag)
		{
			return;
		}
		if(Combat->CombatState == ECombatState::ECS_Unoccupied) ServerEquipButtonPressed();

		bool bSwap = 
			Combat->ShouldSwapWeapons() &&
			!HasAuthority() && Combat->CombatState == ECombatState::ECS_Unoccupied &&
			OverlappingWeapon == nullptr;

		if (bSwap)
		{
			PlaySwapMontage();	
			Combat->CombatState = ECombatState::ECS_SwapingWeapons;
			bFinishedSwaping = false;
		}
	}
}
void AShooterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon); 
		}
		else
		{
			if (Combat->ShouldSwapWeapons())
			{
				Combat->SwapWeapon();
			}
		}
	}
}
void AShooterCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat && Combat->bHoldingTheFlag)
	{
		return;
	}
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else{
		Crouch();
	}
}
void AShooterCharacter::CalculateYO_Ptich()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from range {270, 360} to { -90, 0} 
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}
void AShooterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	float speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	if (speed == 0.f && !bIsInAir)
	{
		   bRotateRootBone = true;
		   FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		   FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		   AO_Yaw = DeltaAimRotation.Yaw;
		   if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		   {
			   InterpAO_Yaw = AO_Yaw;
		   }
		   bUseControllerRotationYaw = true;
		   TurnInPlace(DeltaTime);
	}
	if (speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 5.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);

		}
	}
	CalculateYO_Ptich();
}
void AShooterCharacter::ChangeFOV()
{
	Changefov = true;
}
float AShooterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}
void AShooterCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	float speed = CalculateSpeed();
	bRotateRootBone = false;
	if (speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	ProxyRotatorLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotatorLastFrame, ProxyRotation).Yaw;
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}
void AShooterCharacter::GernadeButtonPressed()
{
	if (Combat)
	{
		if (Combat->bHoldingTheFlag)
		{
			return;
		}
		Combat->ThrowGernade();
	}
}

void AShooterCharacter::RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	GameMode = GameMode == nullptr ? GetWorld()->GetAuthGameMode<AShooterGameMode>() : GameMode;
	if (bElimed || GameMode == nullptr) return;
	Damage = GameMode->CalculateDamage(InstigatorController, Controller ,Damage);
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHudHealth();
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && !AnimInstance->IsAnyMontagePlaying())
	{
		PlayHitReactMontage();
	}
	if(Health == 0.f) 
	{
			PlayerController = PlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : PlayerController;
			AShooterPlayerController* AttackerContoller = Cast<AShooterPlayerController>(InstigatorController);
			AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(AttackerContoller->GetCharacter());
			if (ShooterCharacter && ShooterCharacter->IsLocallyControlled())
			{
				ShooterCharacter->ShowKillMarker();
			}
			GameMode->PlayerEliminated(this, PlayerController, AttackerContoller);
	}
	ServerShowDamageIndicator(DamagedActor, DamageCauser);
}
void AShooterCharacter::Jump()
{
	if (Combat && Combat->bHoldingTheFlag)
	{
		return;
	}
	if (bDisableGameplay)
	{
		return;
	}
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}
void AShooterCharacter::PollInit()
{
	if (ShooterPlayerState == nullptr)
	{
		ShooterPlayerState = GetPlayerState<AShooterPlayerState>();
		if (ShooterPlayerState)
		{
			OnPlayerStateIntialized();
		}
	}
}

FVector AShooterCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

void AShooterCharacter::TurnInPlace(float DeltaTime)
{
	if(AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
}
void AShooterCharacter::HideCameraWhenCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold && !inFirst)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
		if (MaskMesh)
		{
			MaskMesh->SetVisibility(false);
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if (MaskMesh)
		{
			MaskMesh->SetVisibility(true);
		}
		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
	if(inFirst)
	{
		EPhysBodyOp body = EPhysBodyOp::PBO_None;
		if (IsLocallyControlled())
		{
			GetMesh()->HideBoneByName(FName("neck_01"), body);
			MaskMesh->SetVisibility(false);
		}
	}
}
void AShooterCharacter::OnRep_Health()
{
	UpdateHudHealth();	
	GetHitTarget();
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && !AnimInstance->IsAnyMontagePlaying())
	{
		PlayHitReactMontage();
	}
}

void AShooterCharacter::UpdateDesolveMaterial(float DesolveValue)
{
	if (DynamicDesolveMaterialInstance)
	{
		DynamicDesolveMaterialInstance->SetScalarParameterValue(TEXT("Desolve"), DesolveValue);	
	}
}

void AShooterCharacter::StartDesolve()
{
	DisolveTrack.BindDynamic(this, &AShooterCharacter::UpdateDesolveMaterial);
	if(DesolveCurve && DesolveTimeline)
	{	
		DesolveTimeline->AddInterpFloat(DesolveCurve, DisolveTrack);
		DesolveTimeline->Play();	
	}
}

void AShooterCharacter::SetSpawnPoint()
{
	if (HasAuthority() && ShooterPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStarts);
		TArray<ATeamPlayerStart*> TeamPlayersStarts;
		for (auto Start : PlayerStarts)
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			if (TeamStart && TeamStart->Team == ShooterPlayerState->GetTeam())
			{
				TeamPlayersStarts.Add(TeamStart);
			}
		}
		if (TeamPlayersStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayersStarts[FMath::RandRange(0, TeamPlayersStarts.Num() - 1)];
			SetActorLocationAndRotation(ChosenPlayerStart->GetActorLocation(), ChosenPlayerStart->GetActorRotation());
		}
	}
}

void AShooterCharacter::OnPlayerStateIntialized()
{
	ShooterPlayerState->AddToScore(0.f);
	ShooterPlayerState->AddToDeaths(0);
	SetTeamColor(ShooterPlayerState->GetTeam());
	SetSpawnPoint();

}

void AShooterCharacter::SpawnDefaultWeapon()
{
	GameMode = GameMode == nullptr ? GetWorld()->GetAuthGameMode<AShooterGameMode>() : GameMode;
	UWorld* World = GetWorld();
	if (GameMode && World && !bElimed && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
			StartingWeapon->GetPickUpWidget()->DestroyComponent();
		}
	}
}

void AShooterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickUpWidget(true);
		}
	}
}
void AShooterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickUpWidget(false);
	}
}
bool AShooterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}
bool AShooterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}
void AShooterCharacter::PlayGernadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}
AWeapon* AShooterCharacter::GetEqiuppedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}
void AShooterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;


	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
			if (FireMontage)
			{
			AnimInstance->Montage_Play(FireMontage);
			FName SectionName;
			SectionName = bAiming ? FName("AimRifle") : FName("RifleHip");
			AnimInstance->Montage_JumpToSection(SectionName);
			}
	}
}
void AShooterCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		if (ReloadMontage)
		{			
			AnimInstance->Montage_Play(ReloadMontage);
			FName SectionName;
			switch (Combat->EquippedWeapon->GetWeaponType())
			{
			case EWeaponType::EWT_AssualtRifle:
				SectionName = FName("Rifle");
				break;

			case EWeaponType::EWT_RocketLuncher:
				SectionName = FName("RocketLuncher");
				break;
			case EWeaponType::EWT_Pistol:
				SectionName = FName("Pistol");
				break;
			case EWeaponType::EWT_SMG:
				SectionName = FName("SMG");
				break;
			case EWeaponType::EWT_ShotGun:
				SectionName = FName("Shotgun");
				break;
			case EWeaponType::EWT_SniperRifle:
				SectionName = FName("Sniper");
				break;
			case EWeaponType::EWT_GranadeLuncher:
				SectionName = FName("GranadeLuncher");
				break;
			default:
				break;
			}

			AnimInstance->Montage_JumpToSection(SectionName);
		}
	}
}
void AShooterCharacter::PlayHitReactMontage()
{
if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
void AShooterCharacter::PlayElimMontage()
{

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}
void AShooterCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}
void AShooterCharacter::MulticaseElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;	
	bElimed = true;
	PlayElimMontage();
	//Start DesolveEffect
	if (DesolveMaterialInstance)
	{
		DynamicDesolveMaterialInstance = UMaterialInstanceDynamic::Create(DesolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDesolveMaterialInstance);
		DynamicDesolveMaterialInstance->SetScalarParameterValue(TEXT("Desolve"), 0.55f);
		DynamicDesolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 15.f);
	}
	StartDesolve();
	//Disabling Character Movement
	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
	//Disabling Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (AttachedGernade)
	{
		AttachedGernade->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Spawn elim bot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 100.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint
			
		);
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}
	bool bHideSniperScope = IsLocallyControlled() &&
		Combat &&
		Combat->bAiming && Combat->EquippedWeapon &&
		Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSnipeScopeWidget(false);
	}
	if (MaskMesh)
	{
		MaskMesh->SetVisibility(false);
	}
	//Ammo when elimed
	if (PlayerController)
	{
		PlayerController->SetHudWeaponAmmo(0);
	}
	GetWorldTimerManager().SetTimer(ElimTimer, this, &AShooterCharacter::ElimTimerFinished, ElimDelay);

}

void AShooterCharacter::Elim(bool bPlayerLeftGame)
{
	DropOrDestroyWeapons();
	MulticaseElim(bPlayerLeftGame);
}
void AShooterCharacter::SetTeamColor(ETeam Team)
{
	if (GetMesh() == nullptr && GreenMaterial == nullptr) return;
	switch (Team)
	{
	case ETeam::ET_GreenTeam:
		GetMesh()->SetMaterial(0, GreenMaterial);
		DesolveMaterialInstance = TeamDesolveMaterialInstance;
		break;
	case ETeam::ET_GreyTeam:
		GetMesh()->SetMaterial(0, GreyMaterial);
		DesolveMaterialInstance = TeamDesolveMaterialInstance;
		break;
	case ETeam::ET_NoTeam:
		GetMesh()->SetMaterial(0, GreenMaterial);
		DesolveMaterialInstance = TeamDesolveMaterialInstance;
		break;
	case ETeam::ET_MAX:
		break;
	default:
		break;
	}
}
void AShooterCharacter::Destroyed()
{
	Super::Destroy();	 
	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
	AShooterGameMode* ShooterGameMode = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = ShooterGameMode && ShooterGameMode->GetMatchState() != MatchState::InProgress;
	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}
void AShooterCharacter::ElimTimerFinished()
{
	GameMode = GameMode == nullptr ? GetWorld()->GetAuthGameMode<AShooterGameMode>() : GameMode;
	if (GameMode && !bLeftGame)
	{
		GameMode->RequestRespawn(this, Controller);
	}
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}
void AShooterCharacter::ServerLeaveGame_Implementation()
{
	GameMode = GameMode == nullptr ? GetWorld()->GetAuthGameMode<AShooterGameMode>() : GameMode;
	ShooterPlayerState = ShooterPlayerState == nullptr ? GetPlayerState<AShooterPlayerState>() : ShooterPlayerState;
	if (GameMode && ShooterPlayerState)
	{
		GameMode->PlayerLeftGame(ShooterPlayerState);
	}
}
void AShooterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return;
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Dropped();
	}
}
void AShooterCharacter::DropOrDestroyWeapons()
{
	if (Combat)
	{
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		if (Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
	}
}
void AShooterCharacter::AimButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag)
	{
		return;
	}
	if (bDisableGameplay)
	{
		return;
	}
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}
void AShooterCharacter::AimButtonReleased()
{
	if (Combat && Combat->bHoldingTheFlag)
	{
		return;
	}
	if (bDisableGameplay)
	{
		return;
	}
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}
void AShooterCharacter::FireButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag)
	{
		return;
	}
	if (bDisableGameplay)
	{
		return;
	}
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}
void AShooterCharacter::FireButtonReleased()
{

	if (Combat && Combat->bHoldingTheFlag)
	{
		return;
	}
	if (bDisableGameplay)
	{
		return;
	}
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}
void AShooterCharacter::ReloadButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag)
	{
		return;
	}
	if (bDisableGameplay)
	{
		return;
	}
	if (Combat)
	{
		Combat->Reload();
	}

}
ECombatState AShooterCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}
void AShooterCharacter::SetHoldingTheFlag(bool bHolding)
{
	if (Combat == nullptr) return;
	Combat->bHoldingTheFlag = bHolding;
}
bool AShooterCharacter::IsHoldingTheFlag() const
{
	if (Combat == nullptr) return false;
	return Combat->bHoldingTheFlag;
}
bool AShooterCharacter::IsLocallyReloading()
{
	if (Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}
ETeam AShooterCharacter::GetTeam()
{
	ShooterPlayerState = ShooterPlayerState == nullptr ? GetPlayerState<AShooterPlayerState>() : ShooterPlayerState;
	if (ShooterPlayerState == nullptr) return ETeam::ET_NoTeam;
	return ShooterPlayerState->GetTeam();
}

void AShooterCharacter::ServerShowDamageIndicator(AActor* DamagedActor, AActor* DamageCauser)
{
	MulticastShowDamageIndicator(DamagedActor, DamageCauser);
}

void AShooterCharacter::MulticastShowDamageIndicator_Implementation(AActor* DamagedActor, AActor* DamageCauser)
{
		AShooterCharacter* DamagedCharacter = Cast<AShooterCharacter>(DamagedActor);
	if (DamagedCharacter && DamageCauser)
	{
		FRotator Direction = UKismetMathLibrary::FindLookAtRotation(DamagedActor->GetActorLocation(), DamageCauser->GetActorLocation());
		FRotator PlayerRot = DamagedCharacter->GetControlRotation();
		float AngleDamaged = Direction.Yaw - PlayerRot.Yaw;
		if (DamagedCharacter->IsLocallyControlled())
		{
			DamagedCharacter->Angle = AngleDamaged;
			DamagedCharacter->ShowHitIndicator();
		}
	}
}
