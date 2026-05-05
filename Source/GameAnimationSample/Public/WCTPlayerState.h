#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "WCTTypes.h"
#include "WCTPlayerState.generated.h"

UCLASS()
class GAMEANIMATIONSAMPLE_API AWCTPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    AWCTPlayerState();

    UPROPERTY(ReplicatedUsing = OnRep_WCTScore, BlueprintReadOnly, Category = "WCT|Score")
    int32 WCTScore;

    UPROPERTY(ReplicatedUsing = OnRep_IsChaser, BlueprintReadOnly, Category = "WCT|Role", meta=(AllowPrivateAccess="true", DisplayName="IsChaser"))
    bool bIsChaser;

    UFUNCTION()
    void OnRep_WCTScore();

    UFUNCTION()
    void OnRep_IsChaser();

    void AddScore(int32 Delta);
    UFUNCTION(BlueprintCallable, Category = "WCT|Role")
    void SetIsChaser(bool bNewIsChaser);

    UFUNCTION(BlueprintCallable, Category = "WCT|Role")
    bool GetIsChaser() const { return bIsChaser; }

    UFUNCTION(BlueprintCallable, Category = "WCT|Role")
    bool IsRunner() const { return !bIsChaser; }


    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
