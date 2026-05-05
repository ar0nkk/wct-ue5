#include "WCTGameState.h"
#include "Net/UnrealNetwork.h"

AWCTGameState::AWCTGameState()
{
    RoundPhase = ERoundPhase::WaitingForPlayers;
    CurrentRound = 0;
    TimeRemaining = 0.0f;
}

void AWCTGameState::OnRep_RoundPhase()
{
}

void AWCTGameState::SetRoundPhase(ERoundPhase NewPhase)
{
    RoundPhase = NewPhase;
}

void AWCTGameState::SetCurrentRound(int32 NewRound)
{
    CurrentRound = NewRound;
}

void AWCTGameState::SetTimeRemaining(float NewTime)
{
    TimeRemaining = NewTime;
}

void AWCTGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AWCTGameState, RoundPhase);
    DOREPLIFETIME(AWCTGameState, CurrentRound);
    DOREPLIFETIME(AWCTGameState, TimeRemaining);
}
