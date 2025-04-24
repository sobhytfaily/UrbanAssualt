// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ShooterGameState.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class AShooterPlayerState* ScoringPlayer);
	UPROPERTY(Replicated)
		TArray<AShooterPlayerState*> TopScoringPlayers;
	
	//Teams

	void GreenTeamScores();
	void GreyTeamScores();


	TArray<AShooterPlayerState*> RedTeam;
	TArray<AShooterPlayerState*> BlueTeam;

	UPROPERTY(ReplicatedUsing = OnRep_GreenTeamScore)
	float GreenTeamScore = 0.f;
	UPROPERTY(ReplicatedUsing = OnRep_GreyTeamScore)
	float GreyTeamScore = 0.f;

	UFUNCTION()
		void OnRep_GreenTeamScore();
	UFUNCTION()
		void OnRep_GreyTeamScore();
private:
	float TopScore = 0.f;
	
};
