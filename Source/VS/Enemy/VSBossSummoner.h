#pragma once

#include "CoreMinimal.h"
#include "Enemy/VSBossEnemy.h"
#include "VSBossSummoner.generated.h"

/**
 * 소환 보스.
 * 직접 공격하지 않고, 플레이어와 거리를 유지(카이팅)하며
 * 주기적으로 잡몹을 소환해 물량으로 압박한다.
 * "본체를 빨리 잡아야 한다"는 압박을 만드는 지원형 보스.
 */
UCLASS()
class VS_API AVSBossSummoner : public AVSBossEnemy
{
    GENERATED_BODY()

protected:
    // 카이팅 이동 (소환수 뒤로 도망)
    virtual void MoveTowardPlayer(float DeltaTime) override;

    // 주기적 소환
    virtual void UpdateAttack(float DeltaTime) override;

private:
    void SummonMinions();

    float SummonTimer = BOSS_ATTACK_START_TIME;   // 다음 소환까지 남은 시간(등장 직후엔 유예)
};
