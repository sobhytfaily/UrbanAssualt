// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon/WeaponTypes.h"
#include "Shooter/Team.h"
#include "Weapon.generated.h"
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_intial UMETA(DisplayName = "Intial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_EquippedSecondary UMETA(DisplayName = "EquippedSecondary"),
	//EWS_Reloading UMETA(DisplayName = "Reloading"),
	EWS_MAX UMETA(DisplayName = "DefualtMAX")
};
UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun"),
	EFT_MAX UMETA(DisplayName = "DefualtMAX")
};
UCLASS()
class SHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void SetHudAmmo();
	virtual void Tick(float DeltaTime) override;

	void ShowPickUpWidget(bool BShowWidget);
	virtual void Fire(const FVector& HitTarget);

	virtual void Dropped();
	UPROPERTY(EditAnywhere, Category = "Crosshairs")

		class UTexture2D* CrosshairsCenter;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")

		class UTexture2D* CrosshairsLeft;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")

		class UTexture2D* CrosshairsRight;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")

		class UTexture2D* CrosshairsTop;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")

		class UTexture2D* CrosshairsBottom;


	//Automatic fire
	UPROPERTY(EditAnywhere, Category = "Combat")
		float FireDelay = 0.5f;
	UPROPERTY(EditAnywhere, Category = "Combat")
		bool bAutomatic = true;
	UPROPERTY(EditAnywhere, Category = "Combat")
	class USoundCue* EquipeSound;

	UPROPERTY(EditAnywhere, Category = "Combat")
		class USoundBase* HitMarker;

	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere)
	EFireType FireType;
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;
	FVector TraceEndWithScatter(const FVector& HitTarget);

protected:

	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void OnWeaponEquipped();
	virtual void OnWeaponDropped();
	virtual void OnEquippedSecondary();

	UFUNCTION()
	virtual void OnSphereOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult
	);
	UFUNCTION()
		void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex);
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float distanceToSphere = 80.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
		float sphereRadius = 80.f;


	UPROPERTY(EditAnywhere)
		float Damage = 20.f;

	UPROPERTY(EditAnywhere)
		float HeadShotDamage = 40.f;

	UPROPERTY(Replicated ,EditAnywhere)
		bool bUseServerSideRewind = false;

	UPROPERTY()
		class AShooterCharacter* PlayerCharacter;
	UPROPERTY()
		class AShooterPlayerController* PlayerController;

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);
private:
	UPROPERTY(EditAnywhere)
		float DestroyWeaponTime = 50.f;
	FTimerHandle DestroyWeaponTimer;

	void DestroyWeapon();


	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState,VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;
 	
	UFUNCTION()
	void OnRep_WeaponState();


	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")

	class UAnimationAsset* FireAnimation;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ABulletShell> BulletShellClass;

	//WeaponCrossHairs
	

	// Zoom FOV while Aiming
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	float ZoomedFov = 30.f;
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 Ammo;
	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);
	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);
	UPROPERTY(EditAnywhere, Category = "Ammo")
	int32 MagCapacity;

	//number of unprocessed server requests
	//incremented in spend round and decremented in Client Update Ammo
	int32 Sequence = 0.f;
	
	void SpendRound();


	UPROPERTY(EditAnywhere)
		EWeaponType WeaponType;

	UPROPERTY(EditAnywhere)
		ETeam Team;

public:
	/*
	Enable or disable Custom Depth
	*/
	void EnableCustomDepth(bool bEnable);
	void SetWeaponState(EWeaponState State);
	void AddAmmo(int32 AmmoToAdd);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE UWidgetComponent* GetPickUpWidget() const { return PickupWidget; }
	FORCEINLINE float GetZoomedFov() { return ZoomedFov; }
	FORCEINLINE float GetZoomInterpSpeed() { return ZoomInterpSpeed; }
	FORCEINLINE bool IsEmpty() { return Ammo <= 0; }
	FORCEINLINE bool IsFull() { return Ammo == MagCapacity; }
	FORCEINLINE EWeaponType GetWeaponType() { return WeaponType; }
	FORCEINLINE int32 GetAmmo() { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() { return MagCapacity; }
	FORCEINLINE float GetDamage() { return Damage; }
	FORCEINLINE float GetHeadShotDamage() { return HeadShotDamage; }
	FORCEINLINE ETeam GetTeam() { return Team; }


};
