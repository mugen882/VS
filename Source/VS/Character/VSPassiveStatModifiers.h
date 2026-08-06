#pragma once
#include "CoreMinimal.h"
#include "Common/VSDefine.h"
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

// 각 패시브의 누적 값 상한. Get()이 이 값에 도달하면 더 찍어도 실제 수치가 오르지 않는다.
inline float GetPassiveValueCap(EVSPassiveStatType Key)
{
    switch (Key)
    {
    case EVSPassiveStatType::MoveSpeed:      return MAX_MOVESPEED_BONUS;
    case EVSPassiveStatType::MaxHealth:      return MAX_MAXHEALTH_BONUS;
    case EVSPassiveStatType::PickupRange:    return MAX_PICKUPRANGE_BONUS;
    case EVSPassiveStatType::GlobalDamage:   return MAX_GLOBALDAMAGE_BONUS;
    // GlobalCooldown은 값 상한 대신 IsCooldownSaturated()(MIN_COOLDOWN_TIME 기준)로 판정한다.
    case EVSPassiveStatType::GlobalCooldown: return TNumericLimits<float>::Max();
    default:                                 return TNumericLimits<float>::Max();
    }
}

USTRUCT()
struct FVSPassiveStatModifiers
{
    GENERATED_BODY()

    // StatType → 누적 값. 예: GlobalDamage → 0.30 (=+30%)
    UPROPERTY()
    TMap<EVSPassiveStatType, float> Values;

    // StatType → 획득 횟수(스택). 상한 판정에만 쓴다.
    UPROPERTY()
    TMap<EVSPassiveStatType, int32> Levels;

    float Get(EVSPassiveStatType Key) const
    {
        const float* Found = Values.Find(Key);
        return Found ? *Found : 0.f;
    }

    // 아직 한 번도 안 찍었으면 0
    int32 GetLevel(EVSPassiveStatType Key) const
    {
        const int32* Found = Levels.Find(Key);
        return Found ? *Found : 0;
    }

    bool IsMaxLevel(EVSPassiveStatType Key) const
    {
        return GetLevel(Key) >= MAX_PASSIVE_LEVEL;
    }

    // 상한에 걸리면 아무것도 하지 않고 false. 호출부에서 실패를 구분할 수 있게 한다.
    bool Add(EVSPassiveStatType Key, float Delta)
    {
        if (IsMaxLevel(Key))
            return false;

        Values.FindOrAdd(Key) += Delta;
        Levels.FindOrAdd(Key) += 1;
        return true;
    }
};
