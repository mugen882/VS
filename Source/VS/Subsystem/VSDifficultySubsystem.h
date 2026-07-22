#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "VSDifficultySubsystem.generated.h"

class UVSWaveData;
struct FVSWaveEntry;
class AVSEnemyManager;

DECLARE_MULTICAST_DELEGATE(FOnRunCleared);

UCLASS()
class UVSDifficultySubsystem : public UWorldSubsystem, public FTickableGameObject
{
    GENERATED_BODY()
public:
    // UWorldSubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // 게임모드가 웨이브 정의를 주입 (push)
    void SetWaveData(UVSWaveData* InWaveData);

    float GetElapsedTime() const { return ElapsedTime; }

public:
    FOnRunCleared OnRunCleared;

protected:
    // FTickableGameObject
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    AVSEnemyManager* GetEnemyManager();

    // ElapsedTime 기준으로 CurrentWaveIndex 갱신, 현재 웨이브 반환 (없으면 nullptr)
    const FVSWaveEntry* ResolveCurrentWave();

private:
    float ElapsedTime = 0.f;
    int32 CurrentWaveIndex = 0;
    float SpawnAccumulator = 0.f;

    UPROPERTY()
    TObjectPtr<UVSWaveData> WaveData;   // 게임모드가 주입한 웨이브 정의

    TWeakObjectPtr<AVSEnemyManager> EnemyManager;

    bool bRunCleared = false;
};
