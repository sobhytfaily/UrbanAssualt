// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);	

/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:


	void SetHudHealth(float Health, float MaxHealth);
	virtual void OnPossess(APawn* InPawn) override;
	void SetHudDeaths(int32 Deaths);
	void SetHudScore(float Score);
	void SetHudWeaponAmmo(int32 Ammo);
	void SetHudCarriedAmmo(int32 Ammo);
	void SetHudMatchCountDown(float CountDownTime);
	void SetHudAnnouncementCountDown(float CountDownTime);
	void SetHudGernades(int32 Gernades);
	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	void HideTeamScores();
	void InitTeamScores();
	void SetHudGreenTeamScore(int32 GreenScore);
	void SetHudGreyTeamScore(int32 GreyScore);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void HandleCooldown();
	float SingleTripTime = 0.f;
	virtual float GetServerTime(); // Synced with server world Clock
	FHighPingDelegate HighPingDelegate;

	void BroadcastElim(AShooterPlayerState* Attacker, AShooterPlayerState* Victim);
	void SetHudDamageIndicator(float Angle);
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	void SetHudTime();
	void PollInit();
	//Sync Time Between Server and Client
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerRecieveClientRequest);
	//Deference Between client and server time
	float ServerClientDelta = 0;
	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFreq = 5.f;
	float TimeSyncRunningTime = 0.f;
	virtual void ReceivedPlayer() override; // Synced with server world Clock as soon as possible
	void HandleMatchHasStarted(bool bTeamMatch = false);
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();
	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(FName StateOfMatch, float WarmUp, float Match, float StartingTime, float cooldown);

	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);

	void ShowReturnToMainMenu();

	UFUNCTION(Client, Reliable)
		void ClientElimAnnoucement(AShooterPlayerState* Attacker, AShooterPlayerState* Victim);

	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	UFUNCTION()
	void OnRep_ShowTeamScores();

	FString GetInfoText(const TArray<class AShooterPlayerState*>& Players);
	FString GetTeamsInfoText(class AShooterGameState* ShooterGameState);;

private:


	//Return To MainMenu
	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<class UUserWidget> ReturnToMainMenuWidget;

	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu;

	bool bReturnToMainMenuOpen = false;

	UPROPERTY()
	class AShooterHud* ShooterHUD;
	UPROPERTY()
	class AShooterGameMode* ShooterGameMode;
	float MatchTime = 0.f;
	float WarmUpTime = 0.f;
	float CooldownTime = 0.f;
	float LevelStartingTime = 0.f;
	uint32 CountDownInt = 0;
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;	
	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
		class UCharacterOverlay* CharacterOverlay;

	bool bIntializeCharacterOverlay = false;
	float HudCarriedAmmo;
	bool bIntializeCarriedAmmo;
	float HudWeaponAmmo;
	bool bIntializeWeaponAmmo;
	float HudHealth;
	float HudMaxHealth;
	float HudScore;
	int32 HudDeaths;
	int32 HudGernades;

	float HighPingRunningTime = 0.f;
	UPROPERTY(EditAnywhere)
		float HighPingDuration = 5.f;
	UPROPERTY(EditAnywhere)
		float HighPingAnimationTime = 0.f;
	UPROPERTY(EditAnywhere)
		float CheckPingFrequency = 20.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);
	
	UPROPERTY(EditAnywhere)
		float HighPingThreshold = 120.f;
};
