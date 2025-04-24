// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TurningInPlace.h"
#include "Shooter/interfaces/InteractWithCrosshairInterface.h"
#include "Components/TimelineComponent.h"
#include "Shooter/CombatState.h"
#include "Shooter/Team.h"
#include "ShooterCharacter.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);


UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter, public	IInteractWithCrosshairInterface
{
	GENERATED_BODY()

public:
	AShooterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayHitReactMontage();
	void PlayElimMontage();
	void PlaySwapMontage();
	void Elim(bool bPlayerLeftGame);

	void SetTeamColor(ETeam Team);
	virtual void Destroyed() override;
	UFUNCTION(Netmulticast, Reliable)
	void MulticaseElim(bool bPlayerLeftGame);
	virtual void OnRep_ReplicatedMovement() override;
	UPROPERTY()
	class AShooterPlayerState* ShooterPlayerState;
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSnipeScopeWidget(bool bShowScope);

	UFUNCTION(BlueprintImplementableEvent)
		void ShowHitMarker();

	UFUNCTION(BlueprintImplementableEvent)
		void ShowHeadShotHitMarker();

	UFUNCTION(BlueprintImplementableEvent)
		void ShowKillMarker();
	UFUNCTION(BlueprintImplementableEvent)
		void ShowHitIndicator();

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		float Angle;
	bool inFirst = false;


		void ServerShowDamageIndicator(AActor* DamagedActor, AActor* DamageCauser);


	UFUNCTION(NetMulticast, Reliable)
		void MulticastShowDamageIndicator(AActor* DamagedActor, AActor* DamageCauser);

	UPROPERTY()
		TMap<FName, class UBoxComponent*> HitCollisionBoxies;


	bool bFinishedSwaping = false;

	UFUNCTION(Server, Reliable)
		void ServerLeaveGame();

	FOnLeftGame OnLeftGame;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void UpdateHudHealth();

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();
	void ReloadButtonPressed();
	void CalculateYO_Ptich();
	void AimOffset(float DeltaTime);
	void ChangeFOV();

	void SimProxiesTurn();
	void GernadeButtonPressed();
	void DropOrDestroyWeapon(AWeapon* Weapon);
	void DropOrDestroyWeapons();

	
	UFUNCTION()
	void RecieveDamage(AActor*  DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	virtual void Jump() override;
	UPROPERTY(EditAnywhere, Category = "Combat")
	class UAnimMontage* FireMontage;
	UPROPERTY(EditAnywhere, Category = "Combat")
		UAnimMontage* ReloadMontage;
	UPROPERTY(EditAnywhere, Category = "Combat")
		UAnimMontage* HitReactMontage;
	UPROPERTY(EditAnywhere, Category = "Combat")
		UAnimMontage* ElimMontage;
	UPROPERTY(EditAnywhere, Category = "Combat")
		UAnimMontage* ThrowGrenadeMontage;
	UPROPERTY(EditAnywhere, Category = "Combat")
		UAnimMontage* SwapMontage;
	//Poll Fore any Relavent classes and intialize the hud
	void PollInit();
	void RotateInPlace(float DeltaTime);
	float DeltaTimeFov = 0.f;

	void UpdateHudAmmo();

	//HitBoxes used for server side rewind
	UPROPERTY(EditAnywhere)
		class UBoxComponent* head;
	UPROPERTY(EditAnywhere)
		UBoxComponent* pelvis;
	UPROPERTY(EditAnywhere)
		UBoxComponent* spine_02;
	UPROPERTY(EditAnywhere)
		UBoxComponent* spine_03;
	UPROPERTY(EditAnywhere)
		UBoxComponent* upperarm_l;
	UPROPERTY(EditAnywhere)
		UBoxComponent* upperarm_r;
	UPROPERTY(EditAnywhere)
		UBoxComponent* lowerarm_l;
	UPROPERTY(EditAnywhere)
		UBoxComponent* lowerarm_r;
	UPROPERTY(EditAnywhere)
		UBoxComponent* hand_l;
	UPROPERTY(EditAnywhere)
		UBoxComponent* hand_r;
	UPROPERTY(EditAnywhere)
		UBoxComponent* thigh_l;
	UPROPERTY(EditAnywhere)
		UBoxComponent* thigh_r;
	UPROPERTY(EditAnywhere)
		UBoxComponent* calf_l;
	UPROPERTY(EditAnywhere)
		UBoxComponent* calf_r;
	UPROPERTY(EditAnywhere)
		UBoxComponent* foot_l;
	UPROPERTY(EditAnywhere)
		UBoxComponent* foot_r;
private:

	bool IntializedHud = false;

	UPROPERTY(VisibleAnywhere, Category = Camera)
		class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
		class UCameraComponent* FollowCamera;

	//UPROPERTY(EditAnywhere,  BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	//class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
		class AWeapon* OverlappingWeapon;
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	//Shooter Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere) class UBuffComponent* Buff;
	UPROPERTY(VisibleAnywhere) class ULagCompensationComponent* LagCompensation;
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	float AO_Yaw;
	float AO_Pitch;
	float InterpAO_Yaw;
	FRotator StartingAimRotation;
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);
	void HideCameraWhenCharacterClose();
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 150.f;
	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotatorLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;

	float CalculateSpeed();


	//Player Health
	UPROPERTY(EditAnywhere, Category  = "PlayerStats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health,VisibleAnywhere, Category = "PlayerStats")	
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	class AShooterPlayerController* PlayerController;

	bool bElimed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	void ElimTimerFinished();

	bool bLeftGame = false;

	bool Changefov = false;


	//Desolve Effect
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DesolveTimeline;
	FOnTimelineFloat DisolveTrack;
	UPROPERTY(EditAnywhere)
	UCurveFloat* DesolveCurve;
	UFUNCTION()
	void UpdateDesolveMaterial(float DesolveValue);
	void StartDesolve();

	UPROPERTY(VisibleAnywhere, Category = "Elim") //Dynamic instance that we can change at runtime
	UMaterialInstanceDynamic* DynamicDesolveMaterialInstance;
	UPROPERTY(VisibleAnywhere, Category = "Elim")// Material instance set on the blueprint	used with the dynamic material instance
	UMaterialInstance* DesolveMaterialInstance;

	//Team Colors
	UPROPERTY(EditAnywhere, Category = "Elim")
		UMaterialInstance* GreenMaterial;

	UPROPERTY(EditAnywhere, Category = "Elim")
		UMaterialInstance* GreyMaterial;

	UPROPERTY(EditAnywhere, Category = "Elim")
		UMaterialInstance* TeamDesolveMaterialInstance;

	//ElimBot
	UPROPERTY(EditAnywhere, Category = "Elim")
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere, Category = "Elim")
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere, Category = "Elim")
	class USoundCue* ElimBotSound;
	UPROPERTY(EditAnywhere, Category = "Elim")
		class USoundCue* ElimBotSoundSpawn;

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* MaskMesh;

	//Gernade
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGernade;

	//Default Weapon
	UPROPERTY(EditAnywhere)
		TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY()
		class AShooterGameMode* GameMode;

	void SetSpawnPoint();
	void OnPlayerStateIntialized();


public:
	void SpawnDefaultWeapon();
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	void PlayGernadeMontage();
	FORCEINLINE AShooterCharacter* GetCharacter() { return this; }
	FORCEINLINE float GetAO_Yaw() { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() { return AO_Pitch; }
	AWeapon* GetEqiuppedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FVector GetHitTarget() const;
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimed() const { return bElimed; }
	FORCEINLINE float getHealth() const { return Health; }
	FORCEINLINE float getMaxHealth() const { return MaxHealth; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* getCombatComponent() const { return Combat; }
	FORCEINLINE bool getDisabledGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGernade() const { return AttachedGernade; }
	FORCEINLINE ULagCompensationComponent* GetLagCompansationComponent() const { return LagCompensation; }
	void SetHoldingTheFlag(bool bHolding);
	bool IsHoldingTheFlag() const;
	ETeam GetTeam();
	bool IsLocallyReloading();
};
