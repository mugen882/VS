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

    /**
     * 외부에서 보스를 즉시 사망 처리한다 (치트·일괄 정리용).
     * Destroy()를 직접 부르지 말고 이쪽을 쓸 것. 정상 사망과 동일한 경로를 타므로
     * HUD 상단 체력바·머리 위 체력바·구독자 정리가 전부 함께 이루어진다.
     *
     * @param bInGrantRewards false면 XP 젬 드랍 없이 정리만 한다.
     */
    void Kill(bool bInGrantRewards = true);

    bool IsDead() const { return bDead; }

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

    /**
     * 사망 처리 본체. 파생은 자기 정리(예고선 제거 등) 후 Super를 호출한다.
     * 직접 호출 금지 — 진입점은 ReceiveDamage(체력 0) 또는 Kill()이다.
     */
    virtual void OnDeath();

    /** OnDeath에서 드랍을 지급할지 여부. Kill()이 결정한다. */
    bool ShouldGrantRewards() const { return bGrantRewards; }

    virtual float GetContactDamageMultiplier() const { return 1.f; }

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

    // 보스 머리 위 체력바
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<class UWidgetComponent> HealthBarWidget;

    UPROPERTY()
    TObjectPtr<UVSBossData> Data;

    UPROPERTY()
    TWeakObjectPtr<class AVSEnemyManager> EnemyManager;

private:
    void ApplyTint(const FLinearColor& Tint);

    /** 사망 1회 보장 게이트. 모든 사망 경로가 여기를 거쳐 OnDeath로 들어간다. */
    void HandleDeath(bool bInGrantRewards);

private:
    float Health = 0.f;

    // 사망 처리·브로드캐스트 중복 방지 (피격 사망 / Kill 동시 진입 대비)
    bool bDead = false;

    // HandleDeath가 설정. OnDeath가 드랍 여부를 판단할 때 읽는다
    bool bGrantRewards = true;

    // 발끝에서 머리끝까지의 높이. 1회 계산해 캐시한다.
    float HeadHeight = BOSS_HEADBAR_FALLBACK_HEIGHT;
    // 메시 반지름. 1회 계산해 캐시한다.
    float MeshRadius = BOSS_MESH_RADIUS;

    // 체력바를 화면상 위로 밀 거리 (카메라 up 방향)
    UPROPERTY(EditAnywhere, Category="Boss|HealthBar")
    float ScreenUpOffset = 120.f;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UMaterialInstanceDynamic>> BodyMIDs;
};
