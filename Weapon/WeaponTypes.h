#pragma once

#define TRACE_LENGHT 1000000.f	

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_AssualtRifle UMETA(DisplayName = "Assualt Rifle"),
	EWT_RocketLuncher UMETA(DisplayName = "Rocket Luncher"),
	EWT_Pistol UMETA(DisplayName = "Pistol"),
	EWT_SMG UMETA(DisplayName = "SubMachineGun"),
	EWT_ShotGun UMETA(DisplayName = "ShotGun"),
	EWT_SniperRifle UMETA(DisplayName = "Sniper Rifle"),
	EWT_GranadeLuncher UMETA(DisplayName = "Granade Luncher"),
	EWT_Flag UMETA(DisplayName = "Flag"),

	EWT_MAX UMETA(DisplayName = "DefualtMAX")
};