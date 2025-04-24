// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "MultiplayerSessionsSubsystem.h"


void ALobbyGameMode::PostLogin(APlayerController* newPlayer)
{
	Super::PostLogin(newPlayer);

	int32 numberOfPlayers = GameState.Get()->PlayerArray.Num();
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(Subsystem);

		if (numberOfPlayers == Subsystem->DesiredNumPublicConnections)
		{
			UWorld* world = GetWorld();
			if (world)
			{
				bUseSeamlessTravel = true;
				FString MatchType = Subsystem->DesiredMatchType;
				if (MatchType == "FreeForAll")
				{
					world->ServerTravel(FString("/Game/Maps/ShooterMap?listen"));
				}
				else if (MatchType == "Teams")
				{
					world->ServerTravel(FString("/Game/Maps/Teams?listen"));
				}
				else if (MatchType == "CaptureTheFlag")
				{
					world->ServerTravel(FString("/Game/Maps/CaptureTheFlag?listen"));
				}
			}
		}
	}
	
}
