// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "Shooter/PlayerState/ShooterPlayerState.h"
#include "Shooter/GameState/ShooterGameState.h"
#include "Shooter/PlayerController/ShooterPlayerController.h"
#include "Kismet/GameplayStatics.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamMatch = true;
}

void ATeamsGameMode::PostLogin(APlayerController* newPlayer)
{
	Super::PostLogin(newPlayer);

	AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	if (ShooterGameState)
	{
		AShooterPlayerState* ShooterState = newPlayer->GetPlayerState<AShooterPlayerState>();
		if (ShooterState && ShooterState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (ShooterGameState->BlueTeam.Num() >= ShooterGameState->RedTeam.Num())
			{
				ShooterGameState->RedTeam.AddUnique(ShooterState);
				ShooterState->SetTeam(ETeam::ET_GreenTeam);
			}
			else
			{
				ShooterGameState->BlueTeam.AddUnique(ShooterState);
				ShooterState->SetTeam(ETeam::ET_GreyTeam);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	AShooterPlayerState* ShooterState = Exiting->GetPlayerState<AShooterPlayerState>();
	if (ShooterGameState && ShooterState)
	{
		if (ShooterGameState->RedTeam.Contains(ShooterState))
		{
			ShooterGameState->RedTeam.Remove(ShooterState);
		}
		if (ShooterGameState->BlueTeam.Contains(ShooterState))
		{
			ShooterGameState->BlueTeam.Remove(ShooterState);
		}
	}
}

float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	AShooterPlayerState* AttackerPState = Attacker->GetPlayerState<AShooterPlayerState>();
	AShooterPlayerState* VictimPState = Victim->GetPlayerState<AShooterPlayerState>();
	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;
	if (AttackerPState == VictimPState) return BaseDamage;
	if (AttackerPState->GetTeam() == VictimPState->GetTeam()) return 0.f;
	return BaseDamage;
}



void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	if (ShooterGameState)
	{
		for (auto pState : ShooterGameState->PlayerArray)
		{
			AShooterPlayerState* ShooterState = Cast<AShooterPlayerState>(pState.Get());
			if (ShooterState && ShooterState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (ShooterGameState->BlueTeam.Num() >= ShooterGameState->RedTeam.Num())
				{
					ShooterGameState->RedTeam.AddUnique(ShooterState);
					ShooterState->SetTeam(ETeam::ET_GreenTeam);
				}
				else
				{
					ShooterGameState->BlueTeam.AddUnique(ShooterState);
					ShooterState->SetTeam(ETeam::ET_GreyTeam);
				}
			}
		}
		
	}
}
void ATeamsGameMode::PlayerEliminated(AShooterCharacter* ElimedCharacter, AShooterPlayerController* VictemController, AShooterPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimedCharacter, VictemController, AttackerController);

	AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
	AShooterPlayerState* AttackerPlayerState = AttackerController ? Cast<AShooterPlayerState>(AttackerController->PlayerState) : nullptr;
	if (ShooterGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_GreenTeam)
		{
			ShooterGameState->GreenTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_GreyTeam)
		{
			ShooterGameState->GreyTeamScores();
		}
	}
}