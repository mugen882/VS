#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VSBossData.generated.h"

/**
 * 돌진(Charge) 공격 설정.
 * 돌진 보스(AVSBossCharger)의 상태 머신 파라미터를 한 곳에 묶는다.
 */
USTRUCT(BlueprintType)
struct FVSChargeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category="Charge")
    float TriggerRange = 800.f;    // 이 거리 안에 들어오면 돌진 준비 시작

    UPROPERTY(EditAnywhere, Category="Charge")
    float Cooldown = 5.f;          // 돌진 쿨다운(초)

    UPROPERTY(EditAnywhere, Category="Charge")
    float AimTime = 1.f;           // 조준(멈춤) 시간 — 이때 방향 고정

    UPROPERTY(EditAnywhere, Category="Charge")
    float Speed = 900.f;           // 돌진 속도 (평소 MoveSpeed보다 훨씬 빠르게)

    UPROPERTY(EditAnywhere, Category="Charge")
    float Duration = 1.f;         // 돌진 지속 시간(초)

    UPROPERTY(EditAnywhere, Category="Charge")
    float RecoverTime = 1.2f;      // 돌진 후 회복(멈춤) 시간(초)

    UPROPERTY(EditAnywhere, Category="Charge")
    float DamageMultiplier = 3.f;  // 돌진 중 접촉 데미지 배율
};

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

    // --- 돌진 (Charger에서 사용) ---
    UPROPERTY(EditAnywhere, Category="Boss|Charge")
    FVSChargeConfig Charge;

    UPROPERTY(EditAnywhere, Category="Boss|Visual")
    TObjectPtr<USkeletalMesh> Mesh;

    UPROPERTY(EditAnywhere, Category="Boss|Visual")
    TSubclassOf<UAnimInstance> AnimClass;
};
