// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h" 
#include "Shooter/ShooterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimationAsset.h"
#include "Weapon/BulletShell.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"
#include "Shooter/ShooterComponent/CombatComponent.h"	
#include "Shooter/PlayerController/ShooterPlayerController.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	EnableCustomDepth(true);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpWidget"));
	PickupWidget->SetupAttachment(RootComponent);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}



// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if(AreaSphere)
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}



void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter)
	{
		if (WeaponType == EWeaponType::EWT_Flag && ShooterCharacter->GetTeam() == Team) return;		
		if (ShooterCharacter->IsHoldingTheFlag()) return;
		ShooterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter)
	{
		if (WeaponType == EWeaponType::EWT_Flag && ShooterCharacter->GetTeam() == Team) return;
		if (ShooterCharacter->IsHoldingTheFlag()) return;
		ShooterCharacter->SetOverlappingWeapon(nullptr);
	}
}



void AWeapon::SpendRound()
{
	
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHudAmmo();
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		Sequence++;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	Ammo = ServerAmmo;
	Sequence--;
	Ammo -= Sequence;
	SetHudAmmo();
}
void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHudAmmo();
	ClientAddAmmo(AmmoToAdd);
}
void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AShooterCharacter>(GetOwner()) : PlayerCharacter;
	if (PlayerCharacter && PlayerCharacter->getCombatComponent() && IsFull())
	{
		PlayerCharacter->getCombatComponent()->JumpToShotgunEnd();
	}
	SetHudAmmo();

}

void AWeapon::SetHudAmmo()
{
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AShooterCharacter>(GetOwner()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		PlayerController = PlayerController == nullptr ? Cast<AShooterPlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHudWeaponAmmo(Ammo);
		}
	}
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * distanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, sphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	FVector ToEndLoc = EndLoc - TraceStart;

	return FVector(TraceStart + ToEndLoc * TRACE_LENGHT / ToEndLoc.Size());
}



void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		PlayerCharacter = nullptr;
		PlayerController = nullptr;
	}
	else
	{
		SetHudAmmo();
	}
}


void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnWeaponEquipped();
		break;
	case EWeaponState::EWS_Dropped:
		OnWeaponDropped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	}
}

void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}



void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}


void AWeapon::OnEquippedSecondary()
{
	ShowPickUpWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SMG)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);

	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AShooterCharacter>(GetOwner()) : PlayerCharacter;
	if (PlayerCharacter && bUseServerSideRewind)
	{
		PlayerController = PlayerController == nullptr ? Cast<AShooterPlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController && HasAuthority() && !PlayerController->HighPingDelegate.IsBound())
		{
			PlayerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
} 

void AWeapon::OnWeaponEquipped()
{
	ShowPickUpWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SMG)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	EnableCustomDepth(false);
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AShooterCharacter>(GetOwner()) : PlayerCharacter;
	if (PlayerCharacter && bUseServerSideRewind)
	{
		PlayerController = PlayerController == nullptr ? Cast<AShooterPlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController && HasAuthority() && !PlayerController->HighPingDelegate.IsBound() )
		{
			PlayerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnWeaponDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AShooterCharacter>(GetOwner()) : PlayerCharacter;
	if (PlayerCharacter && bUseServerSideRewind)
	{
		PlayerController = PlayerController == nullptr ? Cast<AShooterPlayerController>(PlayerCharacter->Controller) : PlayerController;
		if (PlayerController && HasAuthority() && PlayerController->HighPingDelegate.IsBound())
		{
			PlayerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
	GetWorldTimerManager().SetTimer(DestroyWeaponTimer,
		this,
		&AWeapon::DestroyWeapon,
		DestroyWeaponTime);
}

void AWeapon::DestroyWeapon()
{
	/*if (WeaponState == EWeaponState::EWS_Dropped)
	{
		Destroy();
	}
	else {

	}*/
}

void AWeapon::ShowPickUpWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (BulletShellClass)
	{
		const USkeletalMeshSocket* AmmoEject = GetWeaponMesh()->GetSocketByName(FName("AmmoEject"));
		if (AmmoEject)
		{
			FTransform SocketTransform = AmmoEject->GetSocketTransform(WeaponMesh);
				UWorld* world = GetWorld();
				if (world)
				{
					world->SpawnActor<ABulletShell>(
						BulletShellClass,
						SocketTransform.GetLocation(),
						SocketTransform.GetRotation().Rotator()
						);
				}
			
		}
	}
	SpendRound();
	
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	PlayerCharacter = nullptr;
	PlayerController = nullptr;
}

