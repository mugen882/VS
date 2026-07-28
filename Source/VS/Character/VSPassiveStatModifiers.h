#pragma once
#include "CoreMinimal.h"
#include "VSPassiveStatModifiers.generated.h"

UENUM(BlueprintType)
enum class EVSPassiveStatType : uint8
{
    MoveSpeed       UMETA(DisplayName="이동 속도"),
    MaxHealth       UMETA(DisplayName="최대 체력"),
    PickupRange     UMETA(DisplayName="획득 범위"),
    GlobalDamage    UMETA(DisplayName="전체 데미지"),
    GlobalCooldown  UMETA(DisplayName="쿨다운 감소"),
};

USTRUCT()
struct FVSPassiveStatModifiers
{
    GENERATED_BODY()

    // StatType → 누적 값. 예: GlobalDamage → 0.30 (=+30%)
    UPROPERTY()
    TMap<EVSPassiveStatType, float> Values;

    float Get(EVSPassiveStatType Key) const
    {
        const float* Found = Values.Find(Key);
        return Found ? *Found : 0.f;
    }

    void Add(EVSPassiveStatType Key, float Delta)
    {
        Values.FindOrAdd(Key) += Delta;
    }
};
