#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Common/VSDefine.h"
#include "VSBossEnemy.generated.h"

class UVSBossData;
class USkeletalMeshComponent;
class AVSGemManager;

class AVSBossEnemy;

/**
 * 매 틱 반복되는 플레이어 조회 결과를 한 번에 담는 값 타입.
 * 모든 보스가 "방향 + 거리"를 똑같이 필요로 하므로 베이스에서 한 번만 계산한다.
 */
struct FVSBossPlayerInfo
{
    APawn* Pawn = nullptr;
    FVector Dir = FVector::ZeroVector;   // 보스 -> 플레이어
    float Dist = 0.f;                    // 2D 거리

    bool IsValid() const { return Pawn != nullptr; }
};

// 보스 사망 알림 (웨이브/HUD가 구독). 어느 보스가 죽었는지 전달
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBossDied, AVSBossEnemy*);
// 체력 변경 알림 (0~1 비율) — 체력바가 구독
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBossHealthChanged, float);
// 피격 알림 (this) — 화면 상단 바가 "마지막 타격 보스"를 추적하는 데 사용
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBossDamaged, AVSBossEnemy*);

/**
 * 보스 베이스. 모든 보스가 공유하는 이동·체력·사망을 처리한다.
 * 공격 패턴은 파생 클래스가 UpdateAttack을 오버라이드해 구현한다.
 * (무기는 컴포지션 strategy, 보스는 상속 — 개체마다 메시·전체가 다르므로)
 */
UCLASS(Abstract)
class VS_API AVSBossEnemy : public AActor
{
    GENERATED_BODY()

public:
    AVSBossEnemy();

    // 스폰 직후 데이터로 초기화 (스포너가 호출)
    void InitBoss(UVSBossData* InData);

    // 무기 투사체가 호출 (보스도 피격 대상) — 타격 통합은 다음 단계
    void ReceiveDamage(float Damage);

    float GetHealthPercent() const;
    UVSBossData* GetData() const { return Data; }

public:
    FOnBossDied OnBossDied;
    FOnBossHealthChanged OnBossHealthChanged;
    FOnBossDamaged OnBossDamaged;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaTime) override;

    // 공통: 플레이어를 향해 이동. 이동 안 하는 보스는 오버라이드로 비움
    virtual void MoveTowardPlayer(float DeltaTime);

    // 파생이 구현하는 공격 패턴
    virtual void UpdateAttack(float DeltaTime) {}

    virtual void OnDeath();

    // --- 파생 보스가 공유하는 행동 블록 ---

    // 플레이어 방향/거리를 한 번에 조회
    FVSBossPlayerInfo QueryPlayer() const;

    // 지정 방향으로 이동 (Speed 단위: cm/s)
    void MoveInDirection(const FVector& Dir, float Speed, float DeltaTime);

    // 카이팅: FleeRange보다 가까우면 물러나고, KeepDistance보다 멀면 다가감
    void ApplyKiting(const FVSBossPlayerInfo& Info, float DeltaTime);

    // 액터를 Dir 방향으로 등속 회전(도/초). 메시 축 보정은 생성자에서 1회만 처리한다
    void FaceDirection(const FVector& Dir, float DegPerSec, float DeltaTime);

    // 접촉 사거리 안이면 초당 데미지 적용
    void ApplyContactDamage(float DeltaTime, float DamageMult = 1.f);

    // 데이터의 선회 속도(도/초). 데이터가 없으면 기본값
    float GetRotateSpeedDeg() const;

    // 체력바를 카메라 기준 "화면상 위"에 배치 (탑다운 대각선 카메라 대응)
    void UpdateHealthBarPosition(float DeltaTime);

    // 메시 바운드로 머리 높이, 반지름을 계산해 캐시 (메시 지정 직후 1회)
    void CacheData();

    float GetMeshRadius() const { return MeshRadius; }

protected:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USkeletalMeshComponent> MeshComp;

    // 머리 위 체력바 (각 보스가 자기 것을 소유. 위젯 클래스는 BP에서 지정)
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class UWidgetComponent> HealthBarWidget;

    UPROPERTY()
    TObjectPtr<UVSBossData> Data;

    UPROPERTY()
    TWeakObjectPtr<class AVSEnemyManager> EnemyManager;

private:
    float Health = 0.f;

    // 발끝에서 머리끝까지의 높이. 1회 계산해 캐시한다.
    float HeadHeight = BOSS_HEADBAR_FALLBACK_HEIGHT;
    // 메시 반지름. 1회 계산해 캐시한다.
    float MeshRadius = BOSS_MESH_RADIUS;

    // 체력바를 화면상 위로 밀 거리 (카메라 up 방향)
    UPROPERTY(EditAnywhere, Category="Boss|HealthBar")
    float ScreenUpOffset = 120.f;
};
