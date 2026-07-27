#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VSEnemyTypeData.generated.h"

UCLASS(BlueprintType)
class VS_API UVSEnemyTypeData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category="Enemy|Stats")
    float MaxHealth = 30.f;

    UPROPERTY(EditAnywhere, Category="Enemy|Stats")
    float MoveSpeed = 200.f;

    UPROPERTY(EditAnywhere, Category="Enemy|Stats")
    float ContactDamage = 10.f;

    // 처치 시 젬으로 획득하는 XP
    UPROPERTY(EditAnywhere, Category="Enemy|Stats")
    int32 XPValue = 1;

    // --- 외형 (단일 ISM에서 인스턴스별로 다양화) ---
    UPROPERTY(EditAnywhere, Category="Enemy|Visual")
    float Scale = 1.f;

    UPROPERTY(EditAnywhere, Category="Enemy|Visual")
    FLinearColor Tint = FLinearColor::White;
};
