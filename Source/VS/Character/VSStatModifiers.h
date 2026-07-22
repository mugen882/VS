#pragma once
#include "CoreMinimal.h"
#include "VSStatModifiers.generated.h"

USTRUCT()
struct FVSStatModifiers
{
    GENERATED_BODY()

    // StatName → 누적 값. 예: "GlobalDamage" → 0.30 (=+30%)
    UPROPERTY()
    TMap<FName, float> Values;

    float Get(FName Key) const
    {
        const float* Found = Values.Find(Key);
        return Found ? *Found : 0.f;
    }

    void Add(FName Key, float Delta)
    {
        Values.FindOrAdd(Key) += Delta;
    }
};