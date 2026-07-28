#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "VSWaveData.generated.h"

class UVSEnemyTypeData;
class AVSBossEnemy;
class UVSBossData;

USTRUCT(BlueprintType)
struct FVSWaveEntry
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere) float StartTime = 0.f;      // 이 웨이브 시작 시각(초)
    UPROPERTY(EditAnywhere) TObjectPtr<UVSEnemyTypeData> EnemyType;
    UPROPERTY(EditAnywhere) float SpawnInterval = 1.f;  // 스폰 간격
    UPROPERTY(EditAnywhere) int32 SpawnPerTick = 3;     // 한 번에 몇 마리
    UPROPERTY(EditAnywhere) float HealthMult = 1.f;     // 이 웨이브 적의 체력 배율 (MaxHealth에 곱하는 값)

    UPROPERTY(EditAnywhere, Category="Boss") TSubclassOf<AVSBossEnemy> BossClass;
    UPROPERTY(EditAnywhere, Category="Boss") TObjectPtr<UVSBossData> BossData;
};

UCLASS(BlueprintType)
class UVSWaveData : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere)
    TArray<FVSWaveEntry> Waves;

    UPROPERTY(EditAnywhere, Category="Difficulty")
    float TotalRunTime = 1.f * 60.f;   // 분 * 초
};