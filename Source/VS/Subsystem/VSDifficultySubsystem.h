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

// 간격 기반 스폰 타이머.
// Interval이 0 이하면 비활성으로 본다.
struct FVSSpawnTimer
{
    float Interval = 0.f;
    float Accumulator = 0.f;

    void SetInterval(float InInterval, bool bResetAccumulator = false)
    {
        Interval = InInterval;
        if (bResetAccumulator)
        {
            Reset();
        }
    }

    void Reset() { Accumulator = 0.f; }

    bool IsActive() const { return Interval > 0.f; }

    // 간격만큼 시간이 쌓였으면 true, 그만큼 차감한다.
    bool TryConsumeInterval(float DeltaTime)
    {
        if (Interval <= 0.f) return false;

        Accumulator += DeltaTime;
        if (Accumulator < Interval) return false;

        Accumulator -= Interval;
        return true;
    }
};

UCLASS()
class UVSDifficultySubsystem : public UWorldSubsystem, public FTickableGameObject
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // 게임모드가 웨이브 정의를 주입
    void SetWaveData(UVSWaveData* InWaveData);

    // 치트 : 보스를 즉시 소환한다
    // WaveIndex < 0 이면 현재 웨이브, SpawnDist <= 0 이면 기본 거리를 쓴다.
    bool SpawnBossNow(int32 WaveIndex = INDEX_NONE, float SpawnDist = -1.f);

    float GetElapsedTime() const { return ElapsedTime; }

    // 처치 수 통계
    void AddKill();
    int32 GetKillCount() const { return KillCount; }

    // 도달 웨이브 (결과 화면 표시용)
    int32 GetCurrentWaveNumber() const { return CurrentWaveIndex + 1; }

    // 목표 생존 시간 (HUD 진행바 계산용)
    float GetTotalRunTime() const;

    void RegisterPlayerCharacter(AVSPlayerCharacter* InCharacter);
    void SetUpgradeSelecting(bool bSelecting) { bUpgradeSelecting = bSelecting; }

    // 치트 : 정규 웨이브 스폰을 멈춘다
    void SetBenchmarkPaused(bool bPaused) { bBenchmarkPaused = bPaused; }

    // 웨이브의 자동 스폰(일반·엘리트·보스)을 전부 봉인한다.
    // SpawnBossNow를 통한 수동 소환은 계속 동작한다.
    void SetWaveSpawnDisabled(bool bDisabled) { bWaveSpawnDisabled = bDisabled; }
    bool IsWaveSpawnDisabled() const { return bWaveSpawnDisabled; }

    void SetPauseGame(bool bPause) { bPauseGame = bPause; }

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

    // ElapsedTime 기준으로 CurrentWaveIndex를 갱신한다
    void AdvanceWave();

    // 아직 보스를 내보내지 않은 웨이브라면 소환한다
    void TrySpawnWaveBoss();

    // 현재 스폰에 쓸 웨이브. 첫 웨이브 StartTime 전이면 nullptr
    const FVSWaveEntry* GetActiveWave() const;

    // 웨이브 설정에 따라 일반·엘리트 적을 스폰한다
    void SpawnWaveEnemies(const FVSWaveEntry& Wave, float DeltaTime);
    void SpawnWaveBoss(const FVSWaveEntry& Wave, float SpawnDist);

    void HandlePlayerDied();

    bool CanSpawn() const { return !bGameOver && !bUpgradeSelecting && !bGameClear && !bBenchmarkPaused && !bPauseGame; }

    void ApplyWaveTimers(const FVSWaveEntry& Wave);

    void ResetRunState();

private:
    bool bBenchmarkPaused = false;
    bool bWaveSpawnDisabled = false;

    float ElapsedTime = 0.f;
    int32 CurrentWaveIndex = 0;
    int32 LastBossWaveIndex = -1;
    FVSSpawnTimer SpawnTimer;
    FVSSpawnTimer EliteTimer;
    int32 KillCount = 0;

    UPROPERTY()
    TObjectPtr<UVSWaveData> WaveData;   // 게임모드가 주입한 웨이브 정의

    TWeakObjectPtr<AVSEnemyManager> EnemyManager;

    bool bGameOver = false;
    bool bUpgradeSelecting = false;
    bool bGameClear = false;
	bool bPauseGame = false;

    TWeakObjectPtr<AVSPlayerCharacter> PlayerCharacter;
};
