
#include "FlagZone.h"
#include "Components/SphereComponent.h"
#include "Shooter/Team.h"
#include "Shooter/Flag.h"
#include "Shooter/CaptureTheFlagGameMode.h"

AFlagZone::AFlagZone()
{
	PrimaryActorTick.bCanEverTick = false;
	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Zone Sphere"));
	SetRootComponent(ZoneSphere);
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();
	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereOverlap);
}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFlag* OverlapingFlag = Cast<AFlag>(OtherActor);
	if (OverlapingFlag && OverlapingFlag->GetTeam() != Team)
	{
		ACaptureTheFlagGameMode* GameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>();
		if (GameMode)
		{
			GameMode->FlagCaptured(OverlapingFlag, this);
		}
		OverlapingFlag->ResetFlag();
	}
}
