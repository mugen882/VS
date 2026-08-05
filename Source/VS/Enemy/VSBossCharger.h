#pragma once

#include "CoreMinimal.h"
#include "Enemy/VSBossEnemy.h"
#include "VSBossCharger.generated.h"

class UDecalComponent;
class UMaterialInstanceDynamic;

// 돌진 보스의 상태
UENUM()
enum class EChargerState : uint8
{
    Chase,    // 추격: 플레이어를 천천히 쫓음
    Aim,      // 조준: 멈춰서 방향 고정 (곧 돌진 신호)
    Charge,   // 돌진: 고정한 방향으로 빠르게 직진
    Recover   // 회복: 돌진 후 잠시 멈춤
};

/**
 * 돌진 보스.
 * 평소엔 추격하다가, 플레이어가 사거리 안에 들어오고 쿨다운이 끝나면
 * 조준(방향 고정) -> 돌진(직진) -> 회복 순서로 행동한다.
 * 돌진 방향은 조준 시점에 고정되므로 플레이어가 옆으로 피할 수 있다.
 */
UCLASS()
class VS_API AVSBossCharger : public AVSBossEnemy
{
    GENERATED_BODY()

public:
    AVSBossCharger();

protected:
    // 베이스의 단순 추격 대신 상태 머신으로 이동을 제어
    virtual void MoveTowardPlayer(float DeltaTime) override;

    virtual void OnDeath() override;

    virtual float GetContactDamageMultiplier() const override;

private:
    void EnterState(EChargerState NewState);

    // --- 텔레그래프 ---
    void SetupTelegraph();               // 데이터 기준 크기/위치 계산 (조준 진입 시 1회)
    void ShowTelegraph(bool bShow);
    void UpdateTelegraph(float Ratio);   // 조준 진행도 0~1

    // 돌진 경로 예고 데칼.
    UPROPERTY(VisibleAnywhere, Category="Telegraph")
    UDecalComponent* TelegraphDecal;

    UPROPERTY(Transient)
    UMaterialInstanceDynamic* TelegraphMID = nullptr;

    EChargerState State = EChargerState::Chase;
    float StateTimer = 0.f;          // 현재 상태 경과 시간
    float CooldownTimer = 0.f;       // 다음 돌진까지 남은 쿨다운
    FVector ChargeDir = FVector::ZeroVector;   // 조준 시점에 고정된 돌진 방향
};
