// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ShooterHud.generated.h"



USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()


public:
	class UTexture2D* CrosshairsCenter;
	 UTexture2D* CrosshairsRight;
	 UTexture2D* CrosshairsLeft;
	 UTexture2D* CrosshairsTop;
	 UTexture2D* CrosshairsBottom;
	 float CrosshairSpread;
	 FLinearColor CrosshairColor;
};
/**
 * 
 */
UCLASS()
class SHOOTER_API AShooterHud : public AHUD
{
	GENERATED_BODY()


public:
	virtual void DrawHUD() override;
	
	UPROPERTY(EditAnywhere, Category = "Players Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;
	UPROPERTY()
	class UCharacterOverlay* CharaterOverlay;

	void AddCaracterOverlay();

	UPROPERTY(EditAnywhere, Category = "Announcement")
	TSubclassOf<UUserWidget> AnnouncmentClass;

	UPROPERTY()
	class UAnnouncement* AnnouncementWidget;
	void AddAnnouncement();

	void AddElimAnnouncement(FString AttackerName, FString VictimName);
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class APlayerController* OwningPlayer;

	FHUDPackage HUDPackage;
	void DrawCrosshair(UTexture2D* Texture, FVector2D viewportCenter, FVector2D Spread, FLinearColor CrosshairColor);
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncement> ElimAnnoucementClass;

	UPROPERTY(EditAnywhere)
		float ElimAnnouncementTime = 2.5f;


	UFUNCTION()
		void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove);

	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages;
public:
	float Deltatime = 0.f;
	FORCEINLINE void setHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
