// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "ShooterPlayerController.h"
#include "Shooter/Announcement.h"
#include "Shooter/HUD/CharacterOverlay.h"
#include "Shooter/ShooterHud.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Shooter/ShooterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Shooter/ShooterGameMode.h"
#include "Shooter/HUD/Announcement.h"
#include "Shooter/HUD/ReturnToMainMenu.h"
#include "Kismet/GameplayStatics.h"
#include "Shooter/ShooterComponent/CombatComponent.h"
#include "Shooter/GameState/ShooterGameState.h"
#include "Components/Image.h"
#include "Shooter/PlayerState/ShooterPlayerState.h"

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ShooterHUD = Cast<AShooterHud>(GetHUD());
	ServerCheckMatchState();
}

void AShooterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFreq)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
	if (!bShowTeamScores)
	{
		HideTeamScores();
	}
	SetHudTime();
	PollInit();
	CheckPing(DeltaTime);
}


void AShooterPlayerController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			if (PlayerState->GetPing() * 4 > HighPingThreshold)
			{
				HighPingWarning();
				HighPingAnimationTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}
	bool bHighPingAnimationPlaying = ShooterHUD &&
		ShooterHUD->CharaterOverlay &&
		ShooterHUD->CharaterOverlay->HighPingAnimation &&
		ShooterHUD->CharaterOverlay->IsAnimationPlaying(ShooterHUD->CharaterOverlay->HighPingAnimation);
	if (bHighPingAnimationPlaying)
	{
		HighPingAnimationTime += DeltaTime;
		if (HighPingAnimationTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}
void AShooterPlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidget == nullptr) return;
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			ReturnToMainMenu->MenuSetup(); 
		}
		else
		{
			ReturnToMainMenu->MenuTearDown();
		}
	}
}
void AShooterPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}



void AShooterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}	
void AShooterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterPlayerController, MatchState);
	DOREPLIFETIME(AShooterPlayerController, bShowTeamScores);
}

void AShooterPlayerController::HandleCooldown()
{
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	if (ShooterHUD)
	{
		ShooterHUD->CharaterOverlay->RemoveFromParent();
		if (
			ShooterHUD->AnnouncementWidget &&
			ShooterHUD->AnnouncementWidget->AnnouncementText &&
			ShooterHUD->AnnouncementWidget->infoText
			)
		{
			AShooterGameState* ShooterGameState = Cast<AShooterGameState>(UGameplayStatics::GetGameState(this));
			AShooterPlayerState* CurrentPlayerState = GetPlayerState<AShooterPlayerState>();
			ShooterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			ShooterHUD->AnnouncementWidget->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			if (ShooterGameState && CurrentPlayerState)
			{
				TArray<AShooterPlayerState*> TopPlayers = ShooterGameState->TopScoringPlayers;
				FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(ShooterGameState) : GetInfoText(TopPlayers);
				ShooterHUD->AnnouncementWidget->infoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	AShooterCharacter* shooterCharacter = Cast<AShooterCharacter>(GetPawn());
	if (shooterCharacter && shooterCharacter->getCombatComponent())
	{
		shooterCharacter->bDisableGameplay = true;
		shooterCharacter->getCombatComponent()->FireButtonPressed(false);
	}


}

FString AShooterPlayerController::GetInfoText(const TArray<class AShooterPlayerState*>& Players)
{
	AShooterPlayerState* CurrentPlayerState = GetPlayerState<AShooterPlayerState>();
	if (CurrentPlayerState == nullptr) return FString();
	FString InfoTextString;
	if (Players.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == CurrentPlayerState)
	{
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (Players.Num() == 1)
	{
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		InfoTextString = Announcement::PlayersTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}
	return InfoTextString;
}

FString AShooterPlayerController::GetTeamsInfoText(AShooterGameState* ShooterGameState)
{
	if (ShooterGameMode == nullptr) return FString();
	FString InfoTextString;

	const int32 GreenTeamScore = ShooterGameState->GreenTeamScore;
	const int32 GreyTeamScore = ShooterGameState->GreyTeamScore;

	if (GreenTeamScore == 0 && GreyTeamScore == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (GreenTeamScore == GreyTeamScore)
	{
		InfoTextString = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::GreenTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::GreyTeam);
		InfoTextString.Append(TEXT("\n"));
	}
	else if (GreenTeamScore > GreyTeamScore)
	{
		InfoTextString = Announcement::GreenTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::GreenTeam, GreenTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::GreyTeam, GreyTeamScore));
	}
	else if (GreenTeamScore < GreyTeamScore)
	{
		InfoTextString = Announcement::GreyTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::GreyTeam, GreyTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::GreenTeam, GreenTeamScore));
	}
	return InfoTextString; 
}

void AShooterPlayerController::BroadcastElim(AShooterPlayerState* Attacker, AShooterPlayerState* Victim)
{
	ClientElimAnnoucement(Attacker, Victim);
}
void AShooterPlayerController::SetHudDamageIndicator(float Angle)
{
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD &&
		ShooterHUD->CharaterOverlay &&
		ShooterHUD->CharaterOverlay->DeathAmount &&
		ShooterHUD->CharaterOverlay->DamageIndicator;
	if (bHudValid)
	{
		ShooterHUD->CharaterOverlay->DamageIndicator->SetRenderTransformAngle(Angle);
	}
}
void AShooterPlayerController::ClientElimAnnoucement_Implementation(AShooterPlayerState* Attacker, AShooterPlayerState* Victim)
{
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
		if (ShooterHUD)
		{
			if (Attacker == Self && Victim != Self)
			{
				ShooterHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self)
			{
				ShooterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "you");
				return;
			}
			if (Attacker == Victim && Attacker == Self)
			{
				ShooterHUD->AddElimAnnouncement("You", "yourself");
				return;
			}
			if (Self != Attacker && Attacker == Victim)
			{
				ShooterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "themselves");
				return;
			}
			
			ShooterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn);
	if (ShooterCharacter)
	{
		SetHudHealth(ShooterCharacter->getHealth(), ShooterCharacter->getMaxHealth());
	}
}

void AShooterPlayerController::SetHudDeaths(int32 Deaths)
{
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD &&
		ShooterHUD->CharaterOverlay &&
		ShooterHUD->CharaterOverlay->DeathAmount;
	if (bHudValid)
	{
		FString DeathText = FString::Printf(TEXT("%d"), Deaths);
		ShooterHUD->CharaterOverlay->DeathAmount->SetText(FText::FromString(DeathText));
	}
	else
	{
		bIntializeCharacterOverlay = true;
		HudDeaths = Deaths;
	}
}

void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;

	InputComponent->BindAction("Quit", IE_Pressed, this, &AShooterPlayerController::ShowReturnToMainMenu);
}

void AShooterPlayerController::SetHudHealth(float Health, float MaxHealth)
{
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD &&
		ShooterHUD->CharaterOverlay &&
		ShooterHUD->CharaterOverlay->HealthBar &&
		ShooterHUD->CharaterOverlay->HealthText;
	if (bHudValid)
	{
		const float HealthPercent = Health / MaxHealth;
		ShooterHUD->CharaterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		ShooterHUD->CharaterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bIntializeCharacterOverlay = true;
		HudHealth = Health;
		HudMaxHealth = MaxHealth;
	}
}

void AShooterPlayerController::SetHudScore(float Score)
{
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD &&
		ShooterHUD->CharaterOverlay &&
		ShooterHUD->CharaterOverlay->ScoreAmount;
	if (bHudValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		ShooterHUD->CharaterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bIntializeCharacterOverlay = true;
		HudScore = Score;
	}
}

void AShooterPlayerController::SetHudWeaponAmmo(int32 Ammo)
{
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD &&
		ShooterHUD->CharaterOverlay &&
		ShooterHUD->CharaterOverlay->WeaponAmmoAmount;
	if (bHudValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		ShooterHUD->CharaterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bIntializeWeaponAmmo = true;
		HudWeaponAmmo = Ammo;
	}
}
void AShooterPlayerController::SetHudCarriedAmmo(int32 Ammo)
{
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD &&
		ShooterHUD->CharaterOverlay &&
		ShooterHUD->CharaterOverlay->CarriedWeaponAmmount;
	if (bHudValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		ShooterHUD->CharaterOverlay->CarriedWeaponAmmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bIntializeCarriedAmmo = true;
		HudCarriedAmmo = Ammo;
	}
}

void AShooterPlayerController::SetHudMatchCountDown(float CountDownTime)
{
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD &&
		ShooterHUD->CharaterOverlay &&
		ShooterHUD->CharaterOverlay->MatchCountDownText;
	if (bHudValid)
	{ 
		if (CountDownTime < 0.f)
		{
			ShooterHUD->CharaterOverlay->MatchCountDownText->SetText(FText());
			return;

		}
		int32 Minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 Seconds = CountDownTime - Minutes * 60;
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		ShooterHUD->CharaterOverlay->MatchCountDownText->SetText(FText::FromString(CountDownText));
	}
}

void AShooterPlayerController::SetHudAnnouncementCountDown(float CountDownTime)
{
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD &&
		ShooterHUD->AnnouncementWidget &&
		ShooterHUD->AnnouncementWidget->WarmupTime;
	if (bHudValid)
	{
		if (CountDownTime < 0.f)
		{
			ShooterHUD->AnnouncementWidget->WarmupTime->SetText(FText());
			return;

		}
		int32 Minutes = FMath::FloorToInt(CountDownTime / 60.f);
		int32 Seconds = CountDownTime - Minutes * 60;
		FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		ShooterHUD->AnnouncementWidget->WarmupTime->SetText(FText::FromString(CountDownText));
	}
}

void AShooterPlayerController::SetHudGernades(int32 Gernades)
{
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD &&
		ShooterHUD->CharaterOverlay &&
		ShooterHUD->CharaterOverlay->GernadeText;
	if (bHudValid)
	{
		FString GernadeText = FString::Printf(TEXT("%d"), Gernades);
		ShooterHUD->CharaterOverlay->GernadeText->SetText(FText::FromString(GernadeText));
	}
	else
	{
		HudGernades = Gernades;
	}

}

void AShooterPlayerController::OnMatchStateSet(FName State, bool bTeamMatch)
{
	MatchState = State;
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(bTeamMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AShooterPlayerController::HideTeamScores()
{
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD &&
		ShooterHUD->CharaterOverlay &&
		ShooterHUD->CharaterOverlay->GreenTeamScore &&
		ShooterHUD->CharaterOverlay->GreyTeamScore &&
		ShooterHUD->CharaterOverlay->ScoreSpacerText;
	if (bHudValid)
	{
		ShooterHUD->CharaterOverlay->GreenTeamScore->SetText(FText());
		ShooterHUD->CharaterOverlay->GreyTeamScore->SetText(FText());
		ShooterHUD->CharaterOverlay->ScoreSpacerText->SetText(FText());
	}
}

void AShooterPlayerController::InitTeamScores()
{
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD &&
		ShooterHUD->CharaterOverlay &&
		ShooterHUD->CharaterOverlay->GreenTeamScore &&
		ShooterHUD->CharaterOverlay->GreyTeamScore &&
		ShooterHUD->CharaterOverlay->ScoreSpacerText;
	if (bHudValid)
	{
		FString Zero("0");
		FString Spacer("|");
		ShooterHUD->CharaterOverlay->GreenTeamScore->SetText(FText::FromString(Zero));
		ShooterHUD->CharaterOverlay->GreyTeamScore->SetText(FText::FromString(Zero));
		ShooterHUD->CharaterOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
	}
}

void AShooterPlayerController::SetHudGreenTeamScore(int32 GreenScore)
{
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD &&
		ShooterHUD->CharaterOverlay &&
		ShooterHUD->CharaterOverlay->GreenTeamScore;
	if (bHudValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), GreenScore);
		ShooterHUD->CharaterOverlay->GreenTeamScore->SetText(FText::FromString(ScoreText));
	}
}

void AShooterPlayerController::SetHudGreyTeamScore(int32 GreyScore)
{
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD &&
		ShooterHUD->CharaterOverlay &&
		ShooterHUD->CharaterOverlay->GreyTeamScore;
	if (bHudValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), GreyScore);
		ShooterHUD->CharaterOverlay->GreyTeamScore->SetText(FText::FromString(ScoreText));
	}
}



void AShooterPlayerController::SetHudTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmUpTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmUpTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmUpTime + MatchTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (HasAuthority())
	{
		if (ShooterGameMode == nullptr)
		{
			ShooterGameMode = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this));
			LevelStartingTime = ShooterGameMode->LevelStartingTime;
		}
		ShooterGameMode = ShooterGameMode == nullptr ? Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this)) : ShooterGameMode;
		if (ShooterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(ShooterGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}
	if (CountDownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHudAnnouncementCountDown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHudMatchCountDown(TimeLeft);
		}
	}

	CountDownInt = SecondsLeft;
}

void AShooterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (ShooterHUD && ShooterHUD->CharaterOverlay)
		{
			CharacterOverlay = ShooterHUD->CharaterOverlay;
			if (CharacterOverlay)
			{
				SetHudHealth(HudHealth, HudMaxHealth);
				SetHudScore(HudScore);
				SetHudDeaths(HudDeaths);
				if (bIntializeCarriedAmmo) SetHudCarriedAmmo(HudCarriedAmmo);
				if (bIntializeWeaponAmmo) SetHudWeaponAmmo(HudWeaponAmmo);

				AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetPawn());
				if (ShooterCharacter && ShooterCharacter->getCombatComponent())
				{
					SetHudGernades(ShooterCharacter->getCombatComponent()->GetGernades());
				}
			}
		}
	}
}

float AShooterPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	else return GetWorld()->GetTimeSeconds() + ServerClientDelta;
}

void AShooterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}



void AShooterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AShooterPlayerController::HandleMatchHasStarted(bool bTeamMatch)
{
	if (HasAuthority())
	{
		bShowTeamScores = bTeamMatch;
	}
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	if (ShooterHUD)
	{
		if(ShooterHUD->CharaterOverlay == nullptr)
		{
			ShooterHUD->AddCaracterOverlay();
		}
		if (ShooterHUD->AnnouncementWidget)
		{
			ShooterHUD->AnnouncementWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		if (!HasAuthority()) return;
		if (bTeamMatch)
		{
			InitTeamScores();
		}
		else 
		{
			HideTeamScores();
		}
	}
}

void AShooterPlayerController::HighPingWarning()
{
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD &&
		ShooterHUD->CharaterOverlay &&
		ShooterHUD->CharaterOverlay->HighPingImage &&
		ShooterHUD->CharaterOverlay->HighPingAnimation;
	if (bHudValid)
	{
		ShooterHUD->CharaterOverlay->HighPingImage->SetOpacity(1.f);
		ShooterHUD->CharaterOverlay->PlayAnimation(ShooterHUD->CharaterOverlay->HighPingAnimation, 0.f, 5);
	}
}

void AShooterPlayerController::StopHighPingWarning()
{
	ShooterHUD = ShooterHUD == nullptr ? ShooterHUD = Cast<AShooterHud>(GetHUD()) : ShooterHUD;
	bool bHudValid = ShooterHUD &&
		ShooterHUD->CharaterOverlay &&
		ShooterHUD->CharaterOverlay->HighPingImage &&
		ShooterHUD->CharaterOverlay->HighPingAnimation;
	if (bHudValid)
	{
		ShooterHUD->CharaterOverlay->HighPingImage->SetOpacity(0.f);
		if (ShooterHUD->CharaterOverlay->IsAnimationPlaying(ShooterHUD->CharaterOverlay->HighPingAnimation))
		{
			ShooterHUD->CharaterOverlay->StopAnimation(ShooterHUD->CharaterOverlay->HighPingAnimation);
		}
 	}
}

void AShooterPlayerController::ClientJoinMidGame_Implementation(FName StateOfMatch, float WarmUp, float Match, float StartingTime, float Cooldown)
{
	WarmUpTime = WarmUp;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	CooldownTime = Cooldown;
	OnMatchStateSet(MatchState);
	if (ShooterHUD && MatchState == MatchState::WaitingToStart)
	{
		ShooterHUD->AddAnnouncement();
	}
}

void AShooterPlayerController::ServerCheckMatchState_Implementation()
{
	AShooterGameMode* GameMode = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmUpTime = GameMode->WarmUpTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		CooldownTime = GameMode->CooldownTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(MatchState, WarmUpTime, MatchTime, LevelStartingTime, CooldownTime);
	}
}

void AShooterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTime = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTime);
}

void AShooterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerRecieveClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = (0.5f * RoundTripTime);
	float CurrentServerTime = TimeServerRecieveClientRequest + SingleTripTime;
	ServerClientDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();	
}
