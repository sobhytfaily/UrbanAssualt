
#include "PickUpSpawnPoint.h"
#include "PickUp.h"

APickUpSpawnPoint::APickUpSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APickUpSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	StartPickUpTimer((AActor*)nullptr);
}

void APickUpSpawnPoint::SpawnPickUpTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickUp();
	}
}
void APickUpSpawnPoint::StartPickUpTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnPickUpTimeMin, SpawnPickUpTimeMax);
	GetWorldTimerManager().SetTimer(SpawnPickUpTimer,
		this,
		&APickUpSpawnPoint::SpawnPickUpTimerFinished,
		SpawnTime);
}
void APickUpSpawnPoint::SpawnPickUp()
{
	int32 numPickUpClasses = PickUpClasses.Num();
	if (numPickUpClasses > 0)
	{
		int32 selection = FMath::RandRange(0, numPickUpClasses - 1);
		SpawnedPickUp = GetWorld()->SpawnActor<APickUp>(PickUpClasses[selection], GetActorTransform());
		if (HasAuthority() && SpawnedPickUp)
		{
			SpawnedPickUp->OnDestroyed.AddDynamic(this, &APickUpSpawnPoint::StartPickUpTimer);
		}
	}
}

void APickUpSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

