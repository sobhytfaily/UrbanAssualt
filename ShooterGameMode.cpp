// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameMode.h"
#include "ShooterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Shooter/PlayerState/ShooterPlayerState.h"
#include "Shooter/PlayerController/ShooterPlayerController.h"
#include "Shooter/GameState/ShooterGameState.h"
namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}
AShooterGameMode::AShooterGameMode()
{
	bDelayedStart = true;
}

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}
void AShooterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (MatchState == MatchState::WaitingToStart)
	{
		CountDownTime = WarmUpTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			StartMatch();
		}
		
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountDownTime = WarmUpTime + MatchTime - GetWorld()->GetRealTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountDownTime = CooldownTime + WarmUpTime + MatchTime - GetWorld()->GetRealTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f)
		{
			RestartGame();
		}
	}
}
void AShooterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();
	 
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(*It);
		if (ShooterPlayer)
		{
			ShooterPlayer->OnMatchStateSet(MatchState, bTeamMatch);
		}
	}
}
float AShooterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}
void AShooterGameMode::PlayerEliminated(AShooterCharacter* ElimedCharacter, AShooterPlayerController* VictemController, AShooterPlayerController* AttackerController)
{
	AShooterPlayerState* AttackerPlayerState = AttackerController ? Cast<AShooterPlayerState>(AttackerController->PlayerState) : nullptr;
	AShooterPlayerState* VictemPlayerState = VictemController ? Cast<AShooterPlayerState>(VictemController->PlayerState) : nullptr;
	
	AShooterGameState* ShooterGameState = GetGameState<AShooterGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictemPlayerState && ShooterGameState)
	{
		AttackerPlayerState->AddToScore(1.f);
		ShooterGameState->UpdateTopScore(AttackerPlayerState);
	}
	if (VictemPlayerState)
	{
		VictemPlayerState->AddToDeaths(1);
	}
	if (ElimedCharacter)
	{
		ElimedCharacter->Elim(false);
	}

	for (FConstPlayerControllerIterator it = GetWorld()->GetPlayerControllerIterator(); it; ++it)
	{
		AShooterPlayerController* ShooterPlayer = Cast<AShooterPlayerController>(*it);
		if (ShooterPlayer && AttackerPlayerState && VictemPlayerState)
		{
			ShooterPlayer->BroadcastElim(AttackerPlayerState, VictemPlayerState);
		}
	}
}
void AShooterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> StartActors;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), StartActors);
		int32 Selection = FMath::RandRange(0, StartActors.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, StartActors[Selection]);
	}
}

void AShooterGameMode::PlayerLeftGame(AShooterPlayerState* LeavingPlayer)
{
	if (LeavingPlayer == nullptr) return;
	AShooterGameState* ShooterGameState = GetGameState<AShooterGameState>();
	if (ShooterGameState && ShooterGameState->TopScoringPlayers.Contains(LeavingPlayer))
	{
		ShooterGameState->TopScoringPlayers.Remove(LeavingPlayer);
	}
	AShooterCharacter* CharacterLeaving =  Cast<AShooterCharacter>(LeavingPlayer->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}


