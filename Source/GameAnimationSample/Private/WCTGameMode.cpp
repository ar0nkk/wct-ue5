#include "WCTGameMode.h"
#include "WCTGameState.h"
#include "WCTPlayerState.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AWCTGameMode::AWCTGameMode()
{
    TotalRounds = 16;
    PreRoundSeconds = 8.0f;
    ChasingSeconds = 20.0f;
    RoundEndSeconds = 2.0f;
    RoundPhaseEndTime = 0.0f;
    bMatchStarted = false;

    GameStateClass = AWCTGameState::StaticClass();
    PlayerStateClass = AWCTPlayerState::StaticClass();
}

void AWCTGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        AWCTGameState* WCTGameState = GetWCTGameState();
        if (WCTGameState)
        {
            WCTGameState->SetRoundPhase(ERoundPhase::WaitingForPlayers);
            WCTGameState->SetCurrentRound(0);
            WCTGameState->SetTimeRemaining(0.0f);
        }
    }
}

void AWCTGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    if (!HostController.IsValid() && NewPlayer && NewPlayer->IsLocalController())
    {
        HostController = NewPlayer;
    }

    TryStartMatch();
}

void AWCTGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);
}

void AWCTGameMode::ReportCapture(AController* Capturer, AController* Captured)
{
    if (!HasAuthority())
    {
        return;
    }

    AWCTGameState* WCTGameState = GetWCTGameState();
    if (!WCTGameState || WCTGameState->RoundPhase != ERoundPhase::Chasing)
    {
        return;
    }

    AWCTPlayerState* CapturerState = GetWCTPlayerState(Capturer);
    AWCTPlayerState* CapturedState = GetWCTPlayerState(Captured);
    if (!CapturerState || !CapturedState)
    {
        return;
    }

    if (!CapturerState->bIsChaser)
    {
        return;
    }

    if (CapturedState->bIsChaser)
    {
        return;
    }

    EndRound(Capturer, false);
}

void AWCTGameMode::TryStartMatch()
{
    if (!HasAuthority() || bMatchStarted)
    {
        return;
    }

    if (GetNumPlayers() < 2)
    {
        return;
    }

    bMatchStarted = true;
    StartNextRound();
}

void AWCTGameMode::StartNextRound()
{
    if (!HasAuthority())
    {
        return;
    }

    AWCTGameState* WCTGameState = GetWCTGameState();
    if (!WCTGameState)
    {
        return;
    }

    if (WCTGameState->CurrentRound >= TotalRounds)
    {
        FinishMatch();
        return;
    }

    WCTGameState->SetCurrentRound(WCTGameState->CurrentRound + 1);
    AssignRolesForNewRound();
    BeginPreRound();
}

void AWCTGameMode::BeginPreRound()
{
    AWCTGameState* WCTGameState = GetWCTGameState();
    if (!WCTGameState)
    {
        return;
    }

    WCTGameState->SetRoundPhase(ERoundPhase::PreRound);
    WCTGameState->SetTimeRemaining(PreRoundSeconds);

    if (RunnerController.IsValid())
    {
        SetControllerMovementEnabled(RunnerController.Get(), true);
    }
    if (ChaserController.IsValid())
    {
        SetControllerMovementEnabled(ChaserController.Get(), false);
    }

    RoundPhaseEndTime = GetWorld()->GetTimeSeconds() + PreRoundSeconds;

    GetWorldTimerManager().ClearTimer(PhaseEndTimerHandle);
    GetWorldTimerManager().SetTimer(
        PhaseEndTimerHandle,
        this,
        &AWCTGameMode::BeginChasing,
        PreRoundSeconds,
        false
    );

    GetWorldTimerManager().ClearTimer(TimeRemainingTimerHandle);
    GetWorldTimerManager().SetTimer(
        TimeRemainingTimerHandle,
        this,
        &AWCTGameMode::UpdateTimeRemaining,
        0.1f,
        true
    );
}

void AWCTGameMode::BeginChasing()
{
    AWCTGameState* WCTGameState = GetWCTGameState();
    if (!WCTGameState)
    {
        return;
    }

    WCTGameState->SetRoundPhase(ERoundPhase::Chasing);
    WCTGameState->SetTimeRemaining(ChasingSeconds);

    if (RunnerController.IsValid())
    {
        SetControllerMovementEnabled(RunnerController.Get(), true);
    }
    if (ChaserController.IsValid())
    {
        SetControllerMovementEnabled(ChaserController.Get(), true);
    }

    RoundPhaseEndTime = GetWorld()->GetTimeSeconds() + ChasingSeconds;

    GetWorldTimerManager().ClearTimer(PhaseEndTimerHandle);
    GetWorldTimerManager().SetTimer(
        PhaseEndTimerHandle,
        FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            if (RunnerController.IsValid())
            {
                EndRound(RunnerController.Get(), true);
            }
        }),
        ChasingSeconds,
        false
    );
}

void AWCTGameMode::EndRound(AController* Winner, bool bRunnerSurvived)
{
    AWCTGameState* WCTGameState = GetWCTGameState();
    if (!WCTGameState)
    {
        return;
    }

    GetWorldTimerManager().ClearTimer(PhaseEndTimerHandle);
    GetWorldTimerManager().ClearTimer(TimeRemainingTimerHandle);

    WCTGameState->SetRoundPhase(ERoundPhase::RoundEnd);
    WCTGameState->SetTimeRemaining(0.0f);

    if (bRunnerSurvived)
    {
        AWCTPlayerState* WinnerState = GetWCTPlayerState(Winner);
        if (WinnerState)
        {
            WinnerState->AddScore(1);
        }
    }

    NextRunnerController = Winner;

    if (RunnerController.IsValid())
    {
        SetControllerMovementEnabled(RunnerController.Get(), false);
    }
    if (ChaserController.IsValid())
    {
        SetControllerMovementEnabled(ChaserController.Get(), false);
    }

    if (WCTGameState->CurrentRound >= TotalRounds)
    {
        FinishMatch();
        return;
    }

    GetWorldTimerManager().ClearTimer(RoundTransitionTimerHandle);
    GetWorldTimerManager().SetTimer(
        RoundTransitionTimerHandle,
        this,
        &AWCTGameMode::StartNextRound,
        RoundEndSeconds,
        false
    );
}

void AWCTGameMode::FinishMatch()
{
    AWCTGameState* WCTGameState = GetWCTGameState();
    if (WCTGameState)
    {
        WCTGameState->SetRoundPhase(ERoundPhase::MatchOver);
        WCTGameState->SetTimeRemaining(0.0f);
    }

    if (RunnerController.IsValid())
    {
        SetControllerMovementEnabled(RunnerController.Get(), false);
    }
    if (ChaserController.IsValid())
    {
        SetControllerMovementEnabled(ChaserController.Get(), false);
    }
}

void AWCTGameMode::AssignRolesForNewRound()
{
    RunnerController.Reset();
    ChaserController.Reset();

    if (NextRunnerController.IsValid())
    {
        RunnerController = NextRunnerController;
    }
    else if (HostController.IsValid())
    {
        RunnerController = HostController;
    }
    else
    {
        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
        {
            if (AController* Controller = It->Get())
            {
                RunnerController = Controller;
                break;
            }
        }
    }

    if (RunnerController.IsValid())
    {
        ChaserController = FindOtherController(RunnerController.Get());
    }

    NextRunnerController.Reset();

    if (AWCTPlayerState* RunnerState = GetWCTPlayerState(RunnerController.Get()))
    {
        RunnerState->SetIsChaser(false);
    }
    if (AWCTPlayerState* ChaserState = GetWCTPlayerState(ChaserController.Get()))
    {
        ChaserState->SetIsChaser(true);
    }
}

void AWCTGameMode::UpdateTimeRemaining()
{
    AWCTGameState* WCTGameState = GetWCTGameState();
    if (!WCTGameState)
    {
        return;
    }

    const float Remaining = FMath::Max(0.0f, RoundPhaseEndTime - GetWorld()->GetTimeSeconds());
    WCTGameState->SetTimeRemaining(Remaining);
}

void AWCTGameMode::SetControllerMovementEnabled(AController* Controller, bool bEnabled) const
{
    if (!Controller)
    {
        return;
    }

    Controller->SetIgnoreMoveInput(!bEnabled);

    ACharacter* Character = Cast<ACharacter>(Controller->GetPawn());
    if (!Character)
    {
        return;
    }

    UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
    if (!Movement)
    {
        return;
    }

    if (bEnabled)
    {
        Movement->SetMovementMode(MOVE_Walking);
    }
    else
    {
        Movement->StopMovementImmediately();
        Movement->DisableMovement();
    }
}

AController* AWCTGameMode::FindOtherController(AController* Controller) const
{
    if (!Controller)
    {
        return nullptr;
    }

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        AController* Current = It->Get();
        if (Current && Current != Controller)
        {
            return Current;
        }
    }

    return nullptr;
}

AWCTGameState* AWCTGameMode::GetWCTGameState() const
{
    return GetGameState<AWCTGameState>();
}

AWCTPlayerState* AWCTGameMode::GetWCTPlayerState(AController* Controller) const
{
    return Controller ? Cast<AWCTPlayerState>(Controller->PlayerState) : nullptr;
}
