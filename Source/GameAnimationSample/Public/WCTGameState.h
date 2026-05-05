#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "WCTTypes.h"
#include "WCTGameState.generated.h"

UCLASS()
class GAMEANIMATIONSAMPLE_API AWCTGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    AWCTGameState();

    UPROPERTY(ReplicatedUsing = OnRep_RoundPhase, BlueprintReadOnly, Category = "WCT|Round")
    ERoundPhase RoundPhase;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "WCT|Round")
    int32 CurrentRound;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "WCT|Round")
    float TimeRemaining;

    UFUNCTION()
    void OnRep_RoundPhase();

    void SetRoundPhase(ERoundPhase NewPhase);
    void SetCurrentRound(int32 NewRound);
    void SetTimeRemaining(float NewTime);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
