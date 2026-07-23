#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "VSGameOverWidget.generated.h"

/**
 * 게임오버 결과 화면.
  */
UCLASS()
class VS_API UVSGameOverWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="GameOver")
    void SetupResult(float SurvivalSeconds, int32 KillCount, int32 ReachedLevel, int32 ReachedWave);

public:
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UTextBlock> SurvivalSecText;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UTextBlock> KillCountText;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UTextBlock> ReachedLevelText;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UTextBlock> ReachedWaveText;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UButton> RestartGameButton;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UButton> QuitGameButton;

protected:
    virtual void NativeConstruct() override;

private:
    UFUNCTION()
    void OnRestartClicked();

    UFUNCTION()
    void OnQuitClicked();

    void RestartGame();

    void QuitGame();
};
