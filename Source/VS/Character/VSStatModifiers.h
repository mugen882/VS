#pragma once
#include "CoreMinimal.h"
#include "VSStatModifiers.generated.h"

// 패시브가 수정할 수 있는 스탯 종류. 새 패시브 추가 시 여기에 항목 추가
UENUM(BlueprintType)
enum class EVSStatType : uint8
{
    MoveSpeed       UMETA(DisplayName="이동 속도"),
    MaxHealth       UMETA(DisplayName="최대 체력"),
    PickupRange     UMETA(DisplayName="획득 범위"),
    GlobalDamage    UMETA(DisplayName="전체 데미지"),
    GlobalCooldown  UMETA(DisplayName="쿨다운 감소"),
};

USTRUCT()
struct FVSStatModifiers
{
    GENERATED_BODY()

    // StatType → 누적 값. 예: GlobalDamage → 0.30 (=+30%)
    UPROPERTY()
    TMap<EVSStatType, float> Values;

    float Get(EVSStatType Key) const
    {
        const float* Found = Values.Find(Key);
        return Found ? *Found : 0.f;
    }

    void Add(EVSStatType Key, float Delta)
    {
        Values.FindOrAdd(Key) += Delta;
    }
};
