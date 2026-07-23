#include "Subsystem/VSDifficultySubsystem.h"
#include "Manager/VSEnemyManager.h"
#include "Data/VSWaveData.h"
#include "Kismet/GameplayStatics.h"
#include "Character/VSCharacter.h"

void UVSDifficultySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    ElapsedTime = 0.f;
    SpawnAccumulator = 0.f;
    CurrentWaveIndex = 0;
    KillCount = 0;
    bGameOver = false;
    bUpgradeSelecting = false;
    bGameClear = false;
}

void UVSDifficultySubsystem::SetWaveData(UVSWaveData* InWaveData)
{
    WaveData = InWaveData;
}

AVSEnemyManager* UVSDifficultySubsystem::GetEnemyManager()
{
    if (EnemyManager.IsValid())
        return EnemyManager.Get();

    if (UWorld* World = GetWorld())
    {
        AActor* Found = UGameplayStatics::GetActorOfClass(World, AVSEnemyManager::StaticClass());
        EnemyManager = Cast<AVSEnemyManager>(Found);
    }
    return EnemyManager.Get();
}

const FVSWaveEntry* UVSDifficultySubsystem::ResolveCurrentWave()
{
    if (!WaveData || WaveData->Waves.Num() == 0)
        return nullptr;

    // 경과 시간이 다음 웨이브의 시작 시각을 넘으면 전환
    while (CurrentWaveIndex + 1 < WaveData->Waves.Num()
        && ElapsedTime >= WaveData->Waves[CurrentWaveIndex + 1].StartTime)
    {
        ++CurrentWaveIndex;
    }

    // 아직 첫 웨이브 StartTime 전이면 스폰 안 함
    if (ElapsedTime < WaveData->Waves[CurrentWaveIndex].StartTime)
        return nullptr;

    return &WaveData->Waves[CurrentWaveIndex];
}

void UVSDifficultySubsystem::Tick(float DeltaTime)
{
    if (!CanSpawn()) return;

    ElapsedTime += DeltaTime;

    // 클리어 판정
    if (WaveData && ElapsedTime >= WaveData->TotalRunTime)
    {
        bGameClear = true;
        OnRunCleared.Broadcast();
        return;
    }

    AVSEnemyManager* Mgr = GetEnemyManager();
    if (!Mgr) return;

    const FVSWaveEntry* Wave = ResolveCurrentWave();
    if (!Wave || !Wave->EnemyType) return;

    // 현재 웨이브의 간격으로 스폰
    SpawnAccumulator += DeltaTime;
    while (SpawnAccumulator >= Wave->SpawnInterval)
    {
        SpawnAccumulator -= Wave->SpawnInterval;

        for (int32 i = 0; i < Wave->SpawnPerTick; ++i)
            Mgr->SpawnEnemy(Wave->EnemyType, Wave->HealthMult);
    }
}

TStatId UVSDifficultySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UVSDifficultySubsystem, STATGROUP_Tickables);
}

bool UVSDifficultySubsystem::IsTickable() const
{
    return !IsTemplate() && GetWorld() != nullptr && WaveData != nullptr;
}

void UVSDifficultySubsystem::HandlePlayerDied()
{
    bGameOver = true;
}

void UVSDifficultySubsystem::RegisterPlayerCharacter(AVSCharacter* InCharacter)
{
    if (!InCharacter) return;

    PlayerCharacter = InCharacter;
    InCharacter->OnPlayerDied.AddUObject(this, &UVSDifficultySubsystem::HandlePlayerDied);
}