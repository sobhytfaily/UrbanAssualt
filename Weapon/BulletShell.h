// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletShell.generated.h"

UCLASS()
class SHOOTER_API ABulletShell : public AActor
{
	GENERATED_BODY()
	

protected:
	virtual void BeginPlay() override;
	UFUNCTION()

	virtual void OnHit(UPrimitiveComponent* HitCom, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:	
	ABulletShell();


private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;
	UPROPERTY(EditAnywhere)
	float ShellEjectionImpulse;

	UPROPERTY(EditAnywhere)
	class USoundCue* ShellSound;
public:	

};
