#include "UI/VSResultWidget.h"
#include "Common/VSStringData.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Subsystem/VSDifficultySubsystem.h"

void UVSResultWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (RestartGameButton)
        RestartGameButton->OnClicked.AddDynamic(this, &UVSResultWidget::OnRestartClicked);

    if (QuitGameButton)
        QuitGameButton->OnClicked.AddDynamic(this, &UVSResultWidget::OnQuitClicked);
}

void UVSResultWidget::SetupResult(bool bIsVictory, float InSurvivalSeconds, int32 InKillCount, int32 InReachedLevel, int32 InReachedWave)
{
    if (TitleText)
        TitleText->SetText(bIsVictory ? VictoryTitle : DefeatTitle);

    FNumberFormattingOptions Opt;
    Opt.MinimumFractionalDigits = 2;
    Opt.MaximumFractionalDigits = 2;

    FText StrSurvivalSec = FText::Format(SurvivalSecFormat, FText::AsNumber(InSurvivalSeconds, &Opt));
    SurvivalSecText->SetText(StrSurvivalSec);

    FText StrKillCountSec = FText::Format(KillCountFormat, FText::AsNumber(InKillCount, &Opt));
    KillCountText->SetText(StrKillCountSec);

    FText StrReachedLevel = FText::Format(ReachedLevelFormat, FText::AsNumber(InReachedLevel, &Opt));
    ReachedLevelText->SetText(StrReachedLevel);

    FText StrReachedWave = FText::Format(ReachedWaveFormat, FText::AsNumber(InReachedWave, &Opt));
    ReachedWaveText->SetText(StrReachedWave);
}


void UVSResultWidget::OnRestartClicked()
{
    RestartGame();
}


void UVSResultWidget::OnQuitClicked()
{
    QuitGame();
}

void UVSResultWidget::RestartGame()
{
    // 일시정지 해제
    if (APlayerController* PC = GetOwningPlayer())
        PC->SetPause(false);

    RemoveFromParent();

    const FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(this, /*bRemovePrefixString=*/true);
    UGameplayStatics::OpenLevel(this, FName(*CurrentLevel));
}

void UVSResultWidget::QuitGame()
{
    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, /*bIgnorePlatformRestrictions=*/false);
}