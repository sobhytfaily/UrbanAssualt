// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerState.h"

#include "Shooter/ShooterCharacter.h"
#include "Shooter/PlayerController/ShooterPlayerController.h"
#include "Net/UnrealNetwork.h"

void AShooterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterPlayerState, Deaths);
	DOREPLIFETIME(AShooterPlayerState, Team);
}
void AShooterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHudScore(Score);
		}
	}
}
void AShooterPlayerState::OnRep_Deaths()
{
	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHudDeaths(Deaths);
		}
	}
}
void AShooterPlayerState::AddToDeaths(int32 DeathsAmount)
{
	Deaths += DeathsAmount;
	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHudDeaths(Deaths);
		}
	}
}
void AShooterPlayerState::AddToScore(float ScoreAmount)
{
	Score += ScoreAmount;
	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHudScore(Score);
		}
	}
}

void AShooterPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;

	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetPawn());
	if (ShooterCharacter)
	{
		ShooterCharacter->SetTeamColor(Team);
	}
}
void AShooterPlayerState::OnRep_Team()
{
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetPawn());
	if (ShooterCharacter)
	{
		ShooterCharacter->SetTeamColor(Team);
	}
}