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
        TitleText->SetText(bIsVictory ? VSString::VictoryTitle() : VSString::DefeatTitle());

    // 생존 시간만 소수 2자리, 나머지 정수 스탯은 소수점 없이 표시한다.
    FNumberFormattingOptions Opt;
    Opt.MinimumFractionalDigits = 2;
    Opt.MaximumFractionalDigits = 2;

    if (SurvivalSecText)
        SurvivalSecText->SetText(FText::Format(VSString::SurvivalSecFormat(), FText::AsNumber(InSurvivalSeconds, &Opt)));

    if (KillCountText)
        KillCountText->SetText(FText::Format(VSString::KillCountFormat(), FText::AsNumber(InKillCount)));

    if (ReachedLevelText)
        ReachedLevelText->SetText(FText::Format(VSString::ReachedLevelFormat(), FText::AsNumber(InReachedLevel)));

    if (ReachedWaveText)
        ReachedWaveText->SetText(FText::Format(VSString::ReachedWaveFormat(), FText::AsNumber(InReachedWave)));
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
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->SetPause(false);    // 일시정지 해제
        PC->SetInputMode(FInputModeGameOnly()); // 다시 게임 입력
    }   

    RemoveFromParent();

    const FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(this, /*bRemovePrefixString=*/true);
    UGameplayStatics::OpenLevel(this, FName(*CurrentLevel));
}

void UVSResultWidget::QuitGame()
{
    UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, /*bIgnorePlatformRestrictions=*/false);
}