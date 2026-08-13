#include "ViewModel/VSBossHeadBarViewModel.h"
#include "Enemy/VSBossEnemy.h"
#include "Data/VSBossData.h"

void UVSBossHeadBarViewModel::BindBoss(AVSBossEnemy* InBoss)
{
    if (!InBoss) return;

    // 체력 변경 구독 (Boss→ViewModel 단방향)
    InBoss->OnBossHealthChanged.AddUObject(this, &UVSBossHeadBarViewModel::HandleHealthChanged);

    SetHealthPercent(InBoss->GetHealthPercent());
}

void UVSBossHeadBarViewModel::SetHealthPercent(float V)
{
    UE_MVVM_SET_PROPERTY_VALUE(HealthPercent, V);
}

void UVSBossHeadBarViewModel::HandleHealthChanged(float InPercent)
{
    SetHealthPercent(FMath::Clamp(InPercent, 0.f, 1.f));
}