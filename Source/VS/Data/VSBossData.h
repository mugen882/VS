#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Common/VSDefine.h"
#include "VSBossData.generated.h"

class AVSProjectile;
class UVSEnemyTypeData;

UCLASS(BlueprintType)
class VS_API UVSBossData : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Boss")
    FText DisplayName;   // 체력바에 표시할 이름

    UPROPERTY(EditAnywhere, Category="Boss|Stats")
    float MaxHealth = 1000.f;

    UPROPERTY(EditAnywhere, Category="Boss|Stats")
    float MoveSpeed = 150.f;

    UPROPERTY(EditAnywhere, Category="Boss|Stats")
    float ContactDamage = 20.f;   // 초당 접촉 데미지

    UPROPERTY(EditAnywhere, Category="Boss|Stats")
    int32 XPValue = 100;          // 처치 시 젬 가치

    UPROPERTY(EditAnywhere, Category="Boss|Stats")
    float ContactRange = 150.f;   // 접촉 데미지 판정 거리

    UPROPERTY(EditAnywhere, Category="Boss|Stats", meta=(ClampMin="0", ClampMax="1440"))
    float RotateSpeedDeg = BOSS_ROTATE_SPEED_DEG;   // 선회 속도(도/초). 낮을수록 방향 전환이 굼뜸

    // --- 카이팅 (거리 유지) ---
    UPROPERTY(EditAnywhere, Category="Boss|Kiting")
    float KeepDistance = 1000.f;   // 이보다 멀면 다가감

    UPROPERTY(EditAnywhere, Category="Boss|Kiting")
    float FleeRange = 700.f;       // 이보다 가까우면 물러남

    UPROPERTY(EditAnywhere, Category="Boss|Visual")
    TObjectPtr<USkeletalMesh> Mesh;

    UPROPERTY(EditAnywhere, Category="Boss|Visual")
    TSubclassOf<UAnimInstance> AnimClass;
};

/**
 * 돌진(Charge) 공격 설정.
 * 돌진 보스(AVSBossCharger)의 상태 머신 파라미터를 담는 전용 데이터.
 */
UCLASS(BlueprintType)
class VS_API UVSBossChargerData : public UVSBossData
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Charge")
    float TriggerRange = 800.f;    // 이 거리 안에 들어오면 돌진 준비 시작

    UPROPERTY(EditAnywhere, Category="Charge")
    float Cooldown = 5.f;          // 돌진 쿨다운(초)

    UPROPERTY(EditAnywhere, Category="Charge")
    float AimTime = 1.f;           // 조준(멈춤) 시간 — 이때 방향 고정

    UPROPERTY(EditAnywhere, Category="Charge", meta=(ClampMin="0", ClampMax="1440"))
    float AimRotateSpeedDeg = BOSS_AIM_ROTATE_SPEED_DEG;   // 조준 중 선회 속도(도/초) = 회피 난이도 노브

    UPROPERTY(EditAnywhere, Category="Charge")
    float Speed = 900.f;           // 돌진 속도 (평소 MoveSpeed보다 훨씬 빠르게)

    UPROPERTY(EditAnywhere, Category="Charge")
    float Duration = 1.f;          // 돌진 지속 시간(초)

    UPROPERTY(EditAnywhere, Category="Charge")
    float RecoverTime = 1.2f;      // 돌진 후 회복(멈춤) 시간(초)

    UPROPERTY(EditAnywhere, Category="Charge")
    float DamageMultiplier = 3.f;  // 돌진 중 접촉 데미지 배율
};

/**
 * 원거리 탄막(Ranged) 공격 설정.
 * 원거리 보스(AVSBossRanger)의 전방위 탄막 + 카이팅 파라미터를 담는 전용 데이터.
 */
UCLASS(BlueprintType)
class VS_API UVSBossRangerData : public UVSBossData
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Ranged")
    TSubclassOf<AVSProjectile> ProjectileClass;

    UPROPERTY(EditAnywhere, Category="Ranged")
    float FireInterval = 2.f;       // 탄막 발사 간격(초)

    UPROPERTY(EditAnywhere, Category="Ranged")
    int32 ProjectileCount = 12;     // 전방위 탄 개수 (360도 균등 분할)

    UPROPERTY(EditAnywhere, Category="Ranged")
    float ProjectileDamage = 15.f;

    UPROPERTY(EditAnywhere, Category="Ranged")
    float ProjectileSpeed = 600.f;
};

/**
 * 소환(Summon) 공격 설정.
 * 소환 보스(AVSBossSummoner)가 주기적으로 잡몹을 불러내며 도망 다니는 파라미터.
 */
UCLASS(BlueprintType)
class VS_API UVSBossSummonerData : public UVSBossData
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Summon")
    TObjectPtr<UVSEnemyTypeData> MinionType;   // 소환할 잡몹 타입

    UPROPERTY(EditAnywhere, Category="Summon")
    float SummonInterval = 3.f;      // 소환 간격(초)

    UPROPERTY(EditAnywhere, Category="Summon")
    int32 MinionsPerSummon = 3;      // 한 번에 소환하는 수

    UPROPERTY(EditAnywhere, Category="Summon")
    float MinionHealthMult = 1.f;    // 소환수 체력 배율

    UPROPERTY(EditAnywhere, Category="Summon", meta = (ClampMin = "30", ClampMax = "90"))
    float FrontSummonAngleHalfDeg = 45.f;    // 전방 스폰각도 절반

    UPROPERTY(EditAnywhere, Category="Summon")
    float SummonDist = 300.f;               // 소환 거리
};
