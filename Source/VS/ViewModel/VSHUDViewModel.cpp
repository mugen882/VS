#include "ViewModel/VSHUDViewModel.h"
#include "Character/VSCharacter.h"
#include "Subsystem/VSDifficultySubsystem.h"

void UVSHUDViewModel::BindModels(AVSCharacter* InCharacter, UVSDifficultySubsystem* InDifficulty)
{
    if (InCharacter)
    {
        // 델리게이트 구독 (Model→ViewModel 단방향)
        InCharacter->OnHealthChanged.AddUObject(this, &UVSHUDViewModel::HandleHealthChanged);
        InCharacter->OnXPChanged.AddUObject(this, &UVSHUDViewModel::HandleXPChanged);
        InCharacter->OnLevelChanged.AddUObject(this, &UVSHUDViewModel::HandleLevelChanged);

        // 초기값 반영 (구독 전에 이미 세팅된 상태를 한 번 당겨옴)
        const float MaxHP = InCharacter->MaxHealth > 0.f ? InCharacter->MaxHealth : 1.f;
        SetHealthPercent(InCharacter->CurrentHealth / MaxHP);
        const int32 XPNeed = InCharacter->XPToNextLevel > 0 ? InCharacter->XPToNextLevel : 1;
        SetXPPercent((float)InCharacter->CurrentXP / (float)XPNeed);
        SetLevel(InCharacter->CurrentLevel);
    }

    if (InDifficulty)
    {
        InDifficulty->OnKillCountChanged.AddUObject(this, &UVSHUDViewModel::HandleKillCountChanged);
        InDifficulty->OnTimeChanged.AddUObject(this, &UVSHUDViewModel::HandleTimeChanged);
        InDifficulty->OnTotalRuntimeChanged.AddUObject(this, &UVSHUDViewModel::HandleTotalRunTimeChanged);

        SetKillCount(InDifficulty->GetKillCount());
        HandleTimeChanged(InDifficulty->GetElapsedTime());
    }
}

// --- Setters: 값이 실제로 바뀔 때만 FieldNotify 발생 ---
void UVSHUDViewModel::SetHealthPercent(float V)
{
    UE_MVVM_SET_PROPERTY_VALUE(HealthPercent, V);
}

void UVSHUDViewModel::SetXPPercent(float V)
{
    UE_MVVM_SET_PROPERTY_VALUE(XPPercent, V);
}

void UVSHUDViewModel::SetLevel(int32 V)
{
    UE_MVVM_SET_PROPERTY_VALUE(Level, V);
}

void UVSHUDViewModel::SetKillCount(int32 V)
{
    UE_MVVM_SET_PROPERTY_VALUE(KillCount, V);
}

void UVSHUDViewModel::SetTimeProgress(float V)
{
    UE_MVVM_SET_PROPERTY_VALUE(TimeProgress, V);
}

void UVSHUDViewModel::SetSurvivalTimeText(FText V)
{
    UE_MVVM_SET_PROPERTY_VALUE(SurvivalTimeText, V);
}

void UVSHUDViewModel::HandleHealthChanged(float InPercent)
{
    SetHealthPercent(FMath::Clamp(InPercent, 0.f, 1.f));
}

void UVSHUDViewModel::HandleXPChanged(float InPercent)
{
    SetXPPercent(FMath::Clamp(InPercent, 0.f, 1.f));
}

void UVSHUDViewModel::HandleLevelChanged(int32 InLevel)
{
    SetLevel(InLevel);
}

void UVSHUDViewModel::HandleKillCountChanged(int32 InKills)
{
    SetKillCount(InKills);
}

void UVSHUDViewModel::HandleTimeChanged(float ElapsedSeconds)
{
    // 진행바: 매 프레임 부드럽게
    const float Progress = (TotalRunTime > 0.f)
        ? FMath::Clamp(ElapsedSeconds / TotalRunTime, 0.f, 1.f)
        : 0.f;
    SetTimeProgress(Progress);

    // 텍스트: 초가 바뀔 때만 MM:SS 갱신
    const int32 Whole = FMath::FloorToInt(ElapsedSeconds);
    if (Whole != LastWholeSeconds)
    {
        LastWholeSeconds = Whole;
        const int32 Min = Whole / 60;
        const int32 Sec = Whole % 60;
        const FString Str = FString::Printf(TEXT("%02d:%02d"), Min, Sec);
        SetSurvivalTimeText(FText::FromString(Str));
    }
}

void UVSHUDViewModel::HandleTotalRunTimeChanged(float InTotalRuntime)
{
    TotalRunTime = InTotalRuntime;
}
