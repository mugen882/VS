#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "VSDifficultySubsystem.generated.h"

struct FVSWaveEntry;
class AVSBossEnemy;
class UVSWaveData;
class AVSEnemyManager;
class AVSPlayerCharacter;

DECLARE_MULTICAST_DELEGATE(FOnRunCleared);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnKillCountChanged, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTimeChanged, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTotalRunTimeChanged, float);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnBossSpawned, AVSBossEnemy*);

UCLASS()
class UVSDifficultySubsystem : public UWorldSubsystem, public FTickableGameObject
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // 게임모드가 웨이브 정의를 주입
    void SetWaveData(UVSWaveData* InWaveData);

    float GetElapsedTime() const { return ElapsedTime; }

    // 처치 수 통계
    void AddKill();
    int32 GetKillCount() const { return KillCount; }

    // 도달 웨이브 (1-based, 결과 화면 표시용)
    int32 GetCurrentWaveNumber() const { return CurrentWaveIndex + 1; }

    // 목표 생존 시간 (HUD 진행바 계산용)
    float GetTotalRunTime() const;

    void RegisterPlayerCharacter(AVSPlayerCharacter* InCharacter);
    void SetUpgradeSelecting(bool bSelecting) { bUpgradeSelecting = bSelecting; }

public:
    FOnRunCleared OnRunCleared;
    FOnBossSpawned OnBossSpawned;
    FOnKillCountChanged OnKillCountChanged;
    FOnTimeChanged OnTimeChanged;
    FOnTotalRunTimeChanged OnTotalRuntimeChanged;

protected:
    // FTickableGameObject
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    AVSEnemyManager* GetEnemyManager();

    // ElapsedTime 기준으로 CurrentWaveIndex 갱신, 현재 웨이브 반환
    const FVSWaveEntry* ResolveCurrentWave();
    void SpawnWaveBoss(const FVSWaveEntry& Wave);

    void HandlePlayerDied();

    bool CanSpawn() const { return !bGameOver && !bUpgradeSelecting && !bGameClear && !bBenchmarkPaused; }

public:
    // 성능 측정 중 개체 수가 늘지 않도록 정규 웨이브 스폰을 멈춘다
    void SetBenchmarkPaused(bool bPaused) { bBenchmarkPaused = bPaused; }

private:
    bool bBenchmarkPaused = false;

    float ElapsedTime = 0.f;
    int32 CurrentWaveIndex = 0;
    int32 LastBossWaveIndex = -1;
    float SpawnAccumulator = 0.f;
    float EliteAccumulator = 0.f;
    int32 KillCount = 0;

    UPROPERTY()
    TObjectPtr<UVSWaveData> WaveData;   // 게임모드가 주입한 웨이브 정의

    TWeakObjectPtr<AVSEnemyManager> EnemyManager;

    bool bGameOver = false;
    bool bUpgradeSelecting = false;
    bool bGameClear = false;

    TWeakObjectPtr<AVSPlayerCharacter> PlayerCharacter;
};
