#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "VSBossHeadBarViewModel.generated.h"

class AVSBossEnemy;

/**
 * 보스 머리 위 체력바 뷰모델.
 * 보스와 1:1로 고정
 */
UCLASS(BlueprintType)
class VS_API UVSBossHeadBarViewModel : public UMVVMViewModelBase
{
    GENERATED_BODY()

public:
    // 보스와 연결: 초기 체력·이름 세팅 + 체력 변경 구독
    void BindBoss(AVSBossEnemy* InBoss);

    // --- FieldNotify 프로퍼티 (뷰가 바인딩) ---
    UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Setter)
    float HealthPercent = 1.f;

    // --- Getter / Setter ---
    float GetHealthPercent() const { return HealthPercent; }
    void  SetHealthPercent(float V);

private:
    void HandleHealthChanged(float InPercent);
};
