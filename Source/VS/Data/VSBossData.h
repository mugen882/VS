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

    // --- 원거리 공격 (Charger 등에서 사용) ---
    UPROPERTY(EditAnywhere, Category="Boss|Ranged")
    TSubclassOf<AVSProjectile> ProjectileClass;

    UPROPERTY(EditAnywhere, Category="Boss|Ranged")
    float FireInterval = 3.f;     // 발사 간격(초)

    UPROPERTY(EditAnywhere, Category="Boss|Ranged")
    float ProjectileDamage = 30.f;

    UPROPERTY(EditAnywhere, Category="Boss|Visual")
    TObjectPtr<USkeletalMesh> Mesh;

    UPROPERTY(EditAnywhere, Category="Boss|Visual")
    TSubclassOf<UAnimInstance> AnimClass;
};
