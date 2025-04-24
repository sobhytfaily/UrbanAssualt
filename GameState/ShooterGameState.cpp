// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Shooter/PlayerState/ShooterPlayerState.h"
#include "Shooter/PlayerController/ShooterPlayerController.h"

void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterGameState, TopScoringPlayers);
	DOREPLIFETIME(AShooterGameState, GreenTeamScore);
	DOREPLIFETIME(AShooterGameState, GreyTeamScore);
}

void AShooterGameState::UpdateTopScore(AShooterPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if(ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();

	}

}

void AShooterGameState::GreenTeamScores()
{
	GreenTeamScore++;

	AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (ShooterPlayer)
	{
		ShooterPlayer->SetHudGreenTeamScore(GreenTeamScore);
	}
}

void AShooterGameState::GreyTeamScores()
{
	GreyTeamScore++;

	AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (ShooterPlayer)
	{
		ShooterPlayer->SetHudGreyTeamScore(GreyTeamScore);
	}
}

void AShooterGameState::OnRep_GreenTeamScore()
{
	AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (ShooterPlayer)
	{
		ShooterPlayer->SetHudGreenTeamScore(GreenTeamScore);
	}
}

void AShooterGameState::OnRep_GreyTeamScore()
{
	AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (ShooterPlayer)
	{
		ShooterPlayer->SetHudGreyTeamScore(GreyTeamScore);
	}
}
