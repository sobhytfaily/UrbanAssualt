// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShooterGameMode.generated.h"

namespace MatchState
{
	extern SHOOTER_API const FName Cooldown; //Match duration have been reached display winner

}
/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AShooterGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class AShooterCharacter* ElimedCharacter,
		class AShooterPlayerController* VictemController,
		AShooterPlayerController* AttackerController);

	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);
	void PlayerLeftGame(class AShooterPlayerState* PlayerLeaving);
	UPROPERTY(EditDefaultsOnly)
	float WarmUpTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
		float CooldownTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
		float MatchTime = 120.f;
	float LevelStartingTime = 0.f;

	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);

	bool bTeamMatch = false;	
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
private:
	float CountDownTime = 0.f;
public:
	FORCEINLINE float GetCountdownTime() const { return CountDownTime; }
};
