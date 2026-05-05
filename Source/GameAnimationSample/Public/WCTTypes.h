#pragma once

#include "CoreMinimal.h"
#include "WCTTypes.generated.h"

UENUM(BlueprintType)
enum class ERoundPhase : uint8
{
    WaitingForPlayers UMETA(DisplayName = "Waiting For Players"),
    PreRound UMETA(DisplayName = "Pre-Round"),
    Chasing UMETA(DisplayName = "Chasing"),
    RoundEnd UMETA(DisplayName = "Round End"),
    MatchOver UMETA(DisplayName = "Match Over")
};

