// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
		FVector Location;

	UPROPERTY()
		FRotator Rotation;

	UPROPERTY()
		FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
		float Time;

	UPROPERTY()
		TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
		AShooterCharacter* Character;
};


USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
		bool bHitConfirmed;

	UPROPERTY()
		bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
		TMap<AShooterCharacter*, uint32> HeadShots;
	UPROPERTY()
		TMap<AShooterCharacter*, uint32> BodyShots;

};





UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULagCompensationComponent();
	friend class AShooterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SaveFramePackage();
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	//HitScan
	FServerSideRewindResult ServerSideRewind(
		class AShooterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	);
	UFUNCTION(Server, Reliable)

	void ServerScoreRequest(
		AShooterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	);

	//Projectiles
	

		FServerSideRewindResult ProjectileServerSideRewind(
			AShooterCharacter* HitCharacter,
			const FVector_NetQuantize& TraceStart,
			const FVector_NetQuantize100& InitialVelocity,
			float HitTime
			);

	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(
		AShooterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& IntialVelocity,
		float HitTime
	);

	//Shotgun

	UFUNCTION(Server, Reliable)
		void ShotgunServerScoreRequest(
		const TArray<AShooterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
		);

	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<AShooterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
	);
protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	void MoveBoxes(AShooterCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(AShooterCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(AShooterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	void CacheBoxPosition(AShooterCharacter* HitCharacter, FFramePackage& OutFramePackage);

	FFramePackage GetFrameToCheck(
		AShooterCharacter* HitCharacter,
		float HitTime
		);
	//HitScan

	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		AShooterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);

	//Projectle
	
	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		AShooterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& IntialVelocity,
		float HitTime
	);

	//Shogun Rewinding

	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
	);
private:

	UPROPERTY()
		AShooterCharacter* Character;

	UPROPERTY()
		class AShooterPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

public:


};