#pragma once

#include "CoreMinimal.h"
#include "Enemy/VSBossEnemy.h"
#include "VSBossRanger.generated.h"

/**
 * 원거리 탄막 보스.
 * 플레이어와 일정 거리를 유지(카이팅)하며, 주기적으로 전방위(원형) 탄막을 발사한다.
 * - 너무 가까우면 물러나고, 너무 멀면 다가간다.
 * - 탄막은 360도를 균등 분할해 사방으로 발사 -> 플레이어는 탄 사이 틈으로 회피.
 */
UCLASS()
class VS_API AVSBossRanger : public AVSBossEnemy
{
    GENERATED_BODY()

protected:
    virtual void MoveTowardPlayer(float DeltaTime) override;

    // 주기적 전방위 탄막
    virtual void UpdateAttack(float DeltaTime) override;

private:
    void FireRadialBurst();

    float FireTimer = BOSS_ATTACK_START_TIME;   // 다음 탄막까지 남은 시간(등장 직후엔 유예)
};
