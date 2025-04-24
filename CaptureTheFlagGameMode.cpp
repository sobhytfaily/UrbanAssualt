// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagGameMode.h"
#include "Shooter/Flag.h"
#include "Shooter/CaptureTheFlag/FlagZone.h"
#include "Shooter/GameState/ShooterGameState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(AShooterCharacter* ElimedCharacter, AShooterPlayerController* VictemController, AShooterPlayerController* AttackerController)
{
	AShooterGameMode::PlayerEliminated(ElimedCharacter, VictemController, AttackerController);

}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	AShooterGameState* BGameState = Cast<AShooterGameState>(GameState);
	if (BGameState)
	{
		if (Zone->Team == ETeam::ET_GreenTeam)
		{
			BGameState->GreenTeamScores();
		}
		if (Zone->Team == ETeam::ET_GreyTeam)
		{
			BGameState->GreyTeamScores();
		}
	}
}
