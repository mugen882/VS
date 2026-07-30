#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VSBossData.generated.h"

class AVSProjectile;

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

    // --- 카이팅 (거리 유지) ---
    UPROPERTY(EditAnywhere, Category="Ranged")
    float KeepDistance = 1200.f;    // 이보다 멀면 다가감

    UPROPERTY(EditAnywhere, Category="Ranged")
    float FleeRange = 1000.f;        // 이보다 가까우면 물러남
};
