#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VSHUDViewModel.generated.h"

class AVSCharacter;
class UVSDifficultySubsystem;

/**
 * HUD 뷰모델.
 * Model(캐릭터·서브시스템)의 변경 델리게이트를 구독해 표시용 상태를 갱신한다.
  */
UCLASS(BlueprintType)
class UVSHUDViewModel : public UMVVMViewModelBase
{
    GENERATED_BODY()

public:
    // Model들과 연결 (컨트롤러가 호출): 초기값 세팅 + 델리게이트 구독
    void BindModels(AVSCharacter* InCharacter, UVSDifficultySubsystem* InDifficulty);

    // --- FieldNotify 프로퍼티 (뷰가 바인딩) ---
    UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter)
    float HealthPercent = 1.f;

    UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter)
    float XPPercent = 0.f;

    UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter)
    int32 Level = 1;

    UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter)
    int32 KillCount = 0;

    // 진행바용: 목표 시간 대비 비율 (매 프레임 갱신)
    UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter)
    float TimeProgress = 0.f;

    // 텍스트용: "MM:SS" (초가 바뀔 때만 실제 갱신)
    UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter)
    FText SurvivalTimeText;

    // --- Getter / Setter ---
    float GetHealthPercent() const { return HealthPercent; }
    void  SetHealthPercent(float V);

    float GetXPPercent() const { return XPPercent; }
    void  SetXPPercent(float V);

    int32 GetLevel() const { return Level; }
    void  SetLevel(int32 V);

    int32 GetKillCount() const { return KillCount; }
    void  SetKillCount(int32 V);

    float GetTimeProgress() const { return TimeProgress; }
    void  SetTimeProgress(float V);

    FText GetSurvivalTimeText() const { return SurvivalTimeText; }
    void  SetSurvivalTimeText(FText V);

private:
    // Model 변경 델리게이트 핸들러
    void HandleHealthChanged(float InPercent);
    void HandleXPChanged(float InPercent);
    void HandleLevelChanged(int32 InLevel);
    void HandleKillCountChanged(int32 InKills);
    void HandleTimeChanged(float ElapsedSeconds);
    void HandleTotalRunTimeChanged(float InTotalRuntime);

private:
    float TotalRunTime = 1.f;      // 목표 시간 (진행바 계산용)
    int32 LastWholeSeconds = -1;   // MM:SS 텍스트 갱신을 초 단위로만 하기 위한 캐시
};
