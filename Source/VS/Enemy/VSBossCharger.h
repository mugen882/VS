#pragma once

#include "CoreMinimal.h"
#include "Enemy/VSBossEnemy.h"
#include "VSBossCharger.generated.h"

/**
 * 돌진 + 원거리 투사체 보스.
 * 이동(돌진)은 베이스 MoveTowardPlayer를 그대로 사용하고,
 * UpdateAttack에서 쿨다운마다 플레이어를 향해 투사체를 발사한다.
 */
UCLASS()
class VS_API AVSBossCharger : public AVSBossEnemy
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
    virtual void UpdateAttack(float DeltaTime) override;

private:
    void FireAtPlayer();

    float FireTimer = 0.f;
};
