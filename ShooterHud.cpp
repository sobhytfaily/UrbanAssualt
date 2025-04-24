// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterHud.h"
#include "GameFramework/PlayerController.h"
#include "Components/CanvasPanelSlot.h"
#include "Shooter/HUD/CharacterOverlay.h"
#include "Shooter/ShooterCharacter.h"
#include "Shooter/HUD/ElimAnnouncement.h"
#include "BluePrint/WidgetLayoutLibrary.h"
#include "Components/HorizontalBox.h"
#include "Kismet/GameplayStatics.h"
#include "Shooter/HUD/Announcement.h"
void AShooterHud::BeginPlay()
{
	Super::BeginPlay();
}

void AShooterHud::AddCaracterOverlay()
{
	 APlayerController* playerController = GetOwningPlayerController();
	 if (playerController && CharacterOverlayClass)
	 {
		 CharaterOverlay = CreateWidget<UCharacterOverlay>(playerController, CharacterOverlayClass);
		 CharaterOverlay->AddToViewport();
	 }
}
void AShooterHud::AddAnnouncement()
{
	APlayerController* playerController = GetOwningPlayerController();
	if (playerController && AnnouncmentClass)
	{
		AnnouncementWidget = CreateWidget<UAnnouncement>(playerController, AnnouncmentClass);
		AnnouncementWidget->AddToViewport();
	}
}
void AShooterHud::AddElimAnnouncement(FString AttackerName, FString VictimName)
{
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	if (OwningPlayer && ElimAnnoucementClass)
	{
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayer, ElimAnnoucementClass);
		if (ElimAnnouncementWidget)
		{
			ElimAnnouncementWidget->SetElimAnnouncementText(AttackerName, VictimName);
			ElimAnnouncementWidget->AddToViewport();
			AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OwningPlayer->GetPawn());
			if (ShooterCharacter && AttackerName == "You")
			{
				ShooterCharacter->ShowKillMarker();
			}
			for (UElimAnnouncement* msg : ElimMessages)
			{
				if (msg && msg->AnnouncementBox)
				{
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(msg->AnnouncementBox);
					if (CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						FVector2D NewPosition(CanvasSlot->GetPosition().X,
							Position.Y - CanvasSlot->GetSize().Y
						);
						CanvasSlot->SetPosition(NewPosition);

					}
					
				}
			}
			ElimMessages.Add(ElimAnnouncementWidget);

			FTimerHandle ElimMsgTimer;
			FTimerDelegate ElimMsgDelegate;

			ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);
			GetWorldTimerManager().SetTimer(ElimMsgTimer, ElimMsgDelegate, ElimAnnouncementTime, false);
		}
	}
}
void AShooterHud::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}
void AShooterHud::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
	{
		const float TextureWidth = Texture->GetSizeX();
		const float TextureHeight = Texture->GetSizeY();
		const FVector2D TextureDrawPoint(
			ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
			ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
		);

		DrawTexture(
			Texture,
			TextureDrawPoint.X,
			TextureDrawPoint.Y,
			TextureWidth,
			TextureHeight,
			0.f,
			0.f,
			1.f,
			1.f,
			CrosshairColor
		);
	}



void AShooterHud::DrawHUD()
{
	Super::DrawHUD();
	FVector2D ViewportSize;

	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairsCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsLeft)
		{
			FVector2D Spread(-SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsRight)
		{
			FVector2D Spread(SpreadScaled, 0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsTop)
		{
			FVector2D Spread(0.f, -SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairsBottom)
		{
			FVector2D Spread(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
	}
}
