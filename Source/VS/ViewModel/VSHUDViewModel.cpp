#include "ViewModel/VSHUDViewModel.h"
#include "Character/VSPlayerCharacter.h"
#include "Subsystem/VSDifficultySubsystem.h"
#include "Enemy/VSBossEnemy.h"
#include "Data/VSBossData.h"

void UVSHUDViewModel::BindModels(AVSPlayerCharacter* InCharacter, UVSDifficultySubsystem* InDifficulty)
{
    if (InCharacter)
    {
        // 델리게이트 구독 (Model→ViewModel 단방향).
        InCharacter->OnHealthChanged.RemoveAll(this);
        InCharacter->OnXPChanged.RemoveAll(this);
        InCharacter->OnLevelChanged.RemoveAll(this);
        InCharacter->OnHealthChanged.AddUObject(this, &UVSHUDViewModel::HandleHealthChanged);
        InCharacter->OnXPChanged.AddUObject(this, &UVSHUDViewModel::HandleXPChanged);
        InCharacter->OnLevelChanged.AddUObject(this, &UVSHUDViewModel::HandleLevelChanged);

        // 초기값 반영
        const float MaxHP = InCharacter->MaxHealth > 0.f ? InCharacter->MaxHealth : 1.f;
        SetHealthPercent(InCharacter->CurrentHealth / MaxHP);
        const int32 XPNeed = InCharacter->XPToNextLevel > 0 ? InCharacter->XPToNextLevel : 1;
        SetXPPercent((float)InCharacter->CurrentXP / (float)XPNeed);
        SetLevel(InCharacter->CurrentLevel);
    }

    if (InDifficulty)
    {
        InDifficulty->OnKillCountChanged.RemoveAll(this);
        InDifficulty->OnTimeChanged.RemoveAll(this);
        InDifficulty->OnTotalRuntimeChanged.RemoveAll(this);
        InDifficulty->OnBossSpawned.RemoveAll(this);
        InDifficulty->OnKillCountChanged.AddUObject(this, &UVSHUDViewModel::HandleKillCountChanged);
        InDifficulty->OnTimeChanged.AddUObject(this, &UVSHUDViewModel::HandleTimeChanged);
        InDifficulty->OnTotalRuntimeChanged.AddUObject(this, &UVSHUDViewModel::HandleTotalRunTimeChanged);
        InDifficulty->OnBossSpawned.AddUObject(this, &UVSHUDViewModel::HandleBossSpawned);

        SetKillCount(InDifficulty->GetKillCount());
        HandleTimeChanged(InDifficulty->GetElapsedTime());
    }
}

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
    // 진행바: 매 프레임
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

    RefreshBossBar();
}

void UVSHUDViewModel::HandleTotalRunTimeChanged(float InTotalRuntime)
{
    TotalRunTime = InTotalRuntime;
}

// --- 보스 ---
void UVSHUDViewModel::SetBossBarVisibility(ESlateVisibility V)
{
    UE_MVVM_SET_PROPERTY_VALUE(BossBarVisibility, V);
}

void UVSHUDViewModel::SetBossHealthPercent(float V)
{
    UE_MVVM_SET_PROPERTY_VALUE(BossHealthPercent, V);
}

void UVSHUDViewModel::SetBossName(FText V)
{
    UE_MVVM_SET_PROPERTY_VALUE(BossName, V);
}

void UVSHUDViewModel::HandleBossSpawned(AVSBossEnemy* Boss)
{
    if (!Boss) return;

    // 상단 바 표시는 플레이어가 이 보스를 때렸을 때(HandleBossDamaged) 시작.
    Boss->OnBossDamaged.AddUObject(this, &UVSHUDViewModel::HandleBossDamaged);
    Boss->OnBossDied.AddUObject(this, &UVSHUDViewModel::HandleBossDied);
}

void UVSHUDViewModel::HandleBossDamaged(AVSBossEnemy* Boss)
{
    if (!IsValid(Boss)) return;

    // 마지막 타격 보스를 상단 바 대상으로 (다른 보스였으면 전환)
    if (CurrentTargetBoss.Get() != Boss)
    {
        CurrentTargetBoss = Boss;
        if (UVSBossData* Data = Boss->GetData())
            SetBossName(Data->DisplayName);
        SetBossBarVisibility(ESlateVisibility::Visible);
    }

    // 현재 타겟 보스의 체력 반영
    SetBossHealthPercent(Boss->GetHealthPercent());
}

void UVSHUDViewModel::HandleBossDied(AVSBossEnemy* Boss)
{
    // 죽은 게 현재 상단 바 대상이면 바를 숨긴다
    if (CurrentTargetBoss.Get() == Boss)
    {
        HideBossBar();
    }
}

void UVSHUDViewModel::HideBossBar()
{
    CurrentTargetBoss.Reset();
    SetBossHealthPercent(0.f);
    SetBossBarVisibility(ESlateVisibility::Collapsed);
}

void UVSHUDViewModel::RefreshBossBar()
{
    // 바가 이미 내려가 있으면 할 일이 없다 (보스를 한 번도 안 때린 평상시 경로)
    if (BossBarVisibility == ESlateVisibility::Collapsed) return;

    // 델리게이트가 오지 않은 채 액터만 사라진 경우 — 약참조가 무효화되어 여기서 걸린다
    if (!CurrentTargetBoss.IsValid())
    {
        HideBossBar();
    }
}
