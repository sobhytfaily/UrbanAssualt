#pragma once

UENUM(BlueprintType)
enum class ETeam : uint8
{
	ET_GreenTeam UMETA(DisplayName = "Green Team"),
	ET_GreyTeam UMETA(DisplayName = "Grey Team"),
	ET_NoTeam UMETA(DisplayName = "No Team"),
	ET_MAX UMETA(DisplayName = "Default Max")
};