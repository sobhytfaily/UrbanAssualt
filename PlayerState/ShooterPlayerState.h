// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shooter/Team.h"
#include "GameFramework/PlayerState.h"
#include "ShooterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Deaths();
	void AddToScore(float ScoreAmount);
	void AddToDeaths(int32 DeathsAmount);
	UPROPERTY()
	class AShooterCharacter* Character;
private:
	UPROPERTY()
	class AShooterPlayerController* Controller;
	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths;

	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;	

	UFUNCTION()
	void OnRep_Team();

public:
	FORCEINLINE  ETeam GetTeam() const  { return Team; }
	void SetTeam(ETeam TeamToSet);

};
