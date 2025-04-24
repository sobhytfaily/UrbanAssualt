// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUp.generated.h"

UCLASS()
class SHOOTER_API APickUp : public AActor
{
	GENERATED_BODY()
	
public:	
	APickUp();
	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
private:
	UPROPERTY(EditAnywhere)
		class USphereComponent* OverlapSphere;
	UPROPERTY(EditAnywhere)
		class USoundCue* PickUpSound;
	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* PickUpMesh;
	FTimerHandle BindOverlapTimer;
	float BindOverlapTime = 0.25f;
	void BindOverlapTimerFinished();
public:	


};
