#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VSEnemyTypeData.generated.h"

UCLASS(BlueprintType)
class VS_API UVSEnemyTypeData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // 식별용 (로그/디버그/추후 타입 분기)
    UPROPERTY(EditAnywhere, Category="Enemy")
    FName TypeId = "Default";

    UPROPERTY(EditAnywhere, Category="Enemy|Stats")
    float MaxHealth = 30.f;

    UPROPERTY(EditAnywhere, Category="Enemy|Stats")
    float MoveSpeed = 200.f;

    UPROPERTY(EditAnywhere, Category="Enemy|Stats")
    float ContactDamage = 10.f;

    // 처치 시 젬 가치
    UPROPERTY(EditAnywhere, Category="Enemy|Stats")
    int32 XPValue = 1;

    // --- 외형 (단일 ISM에서 인스턴스별로 다양화) ---
    UPROPERTY(EditAnywhere, Category="Enemy|Visual")
    float Scale = 1.f;

    UPROPERTY(EditAnywhere, Category="Enemy|Visual")
    FLinearColor Tint = FLinearColor::White;
};
