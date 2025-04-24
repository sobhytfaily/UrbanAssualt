// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class SHOOTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	

	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	//Used with Server side Rewind
	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 IntialVelocity;
	
	UPROPERTY(EditAnywhere)
		float IntialSpeed = 15000.f;

	//Only set this for gernades and rockets
	UPROPERTY(EditAnywhere)
		float damage = 20;

	//Doesnt Matter for gernades and rockets
	UPROPERTY(EditAnywhere)
		float headShotDamage = 40.f;
protected:
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* ProjectileMesh;
	UPROPERTY(EditAnywhere)
		class UNiagaraSystem* TrailSystem;
	UPROPERTY()
		class UNiagaraComponent* TrailComponent;
	virtual void BeginPlay() override;
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitCom, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;
	UPROPERTY(VisibleAnywhere) 
	class UProjectileMovementComponent* ProjectileMovementComponent;

	void SpawnTrailSystem();
	void StartDestroyTimer();
	void DestroyTimerFinshed();
	void ExplodeDamage();
	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;
	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;
private:

	FTimerHandle DestroyTimerHandle;
	UPROPERTY(EditAnywhere)
		float DestroyTime = 3.f;
	 
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	class UParticleSystemComponent* TracerComponent;



	
public:	

};
