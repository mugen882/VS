#include "Subsystem/VSDifficultySubsystem.h"
#include "Manager/VSEnemyManager.h"
#include "Data/VSWaveData.h"
#include "Enemy/VSBossEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Character/VSPlayerCharacter.h"

void UVSDifficultySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    ResetRunState();
}

void UVSDifficultySubsystem::ResetRunState()
{
    ElapsedTime = 0.f;
    CurrentWaveIndex = 0;
    LastBossWaveIndex = INDEX_NONE;
    KillCount = 0;
    SpawnTimer = FVSSpawnTimer();
    EliteTimer = FVSSpawnTimer();
    bGameOver = false;
    bUpgradeSelecting = false;
    bGameClear = false;
}

void UVSDifficultySubsystem::SetWaveData(UVSWaveData* InWaveData)
{
    WaveData = InWaveData;

    if (WaveData && WaveData->Waves.IsValidIndex(CurrentWaveIndex))
    {
        ApplyWaveTimers(WaveData->Waves[CurrentWaveIndex]);
    }

    OnTotalRuntimeChanged.Broadcast(GetTotalRunTime());
}

void UVSDifficultySubsystem::AddKill()
{
    ++KillCount;
    OnKillCountChanged.Broadcast(KillCount);
}

float UVSDifficultySubsystem::GetTotalRunTime() const
{
    return WaveData ? WaveData->TotalRunTime : 0.f;
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

void UVSDifficultySubsystem::AdvanceWave()
{
    if (!WaveData || WaveData->Waves.Num() == 0) return;

    // 경과 시간이 다음 웨이브의 시작 시각을 넘으면 전환
    while (CurrentWaveIndex + 1 < WaveData->Waves.Num()
        && ElapsedTime >= WaveData->Waves[CurrentWaveIndex + 1].StartTime)
    {
        ++CurrentWaveIndex;
        ApplyWaveTimers(WaveData->Waves[CurrentWaveIndex]);
    }

    // 봉인 중에는 보스를 내보내지 않으므로 인덱스만 소진시켜 둔다.
    if (bWaveSpawnDisabled)
    {
        LastBossWaveIndex = CurrentWaveIndex;
    }
}

void UVSDifficultySubsystem::TrySpawnWaveBoss()
{
    if (!WaveData || !WaveData->Waves.IsValidIndex(CurrentWaveIndex)) return;
    if (CurrentWaveIndex == LastBossWaveIndex) return;

    LastBossWaveIndex = CurrentWaveIndex;
    SpawnWaveBoss(WaveData->Waves[CurrentWaveIndex], BOSS_SPAWN_DIST);
}

const FVSWaveEntry* UVSDifficultySubsystem::GetActiveWave() const
{
    if (!WaveData || !WaveData->Waves.IsValidIndex(CurrentWaveIndex)) return nullptr;

    // 아직 첫 웨이브 StartTime 전이면 스폰 안 함
    if (ElapsedTime < WaveData->Waves[CurrentWaveIndex].StartTime) return nullptr;

    return &WaveData->Waves[CurrentWaveIndex];
}

bool UVSDifficultySubsystem::SpawnBossNow(int32 WaveIndex, float SpawnDist)
{
    if (!WaveData || WaveData->Waves.Num() == 0) return false;

    const int32 Idx = (WaveIndex >= 0) ? WaveIndex : CurrentWaveIndex;
    if (!WaveData->Waves.IsValidIndex(Idx)) return false;

    const FVSWaveEntry& Wave = WaveData->Waves[Idx];
    if (!Wave.BossClass) return false;

    SpawnWaveBoss(Wave, SpawnDist > 0.f ? SpawnDist : BOSS_SPAWN_DIST);
    return true;
}

void UVSDifficultySubsystem::SpawnWaveBoss(const FVSWaveEntry& Wave, float SpawnDist)
{
    if (!Wave.BossClass) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // 플레이어 주변 링 영역에 스폰
    FVector SpawnLoc = FVector::ZeroVector;
    if (APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0))
    {
        const float Angle = FMath::FRandRange(0.f, 2.f * PI);
        const float Dist = SpawnDist;
        SpawnLoc = Player->GetActorLocation()
            + FVector(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.f);

        SpawnLoc.Z -= Player->GetSimpleCollisionHalfHeight();
    }

    AVSBossEnemy* Boss = World->SpawnActor<AVSBossEnemy>(Wave.BossClass, SpawnLoc, FRotator::ZeroRotator);
    if (Boss)
    {
        Boss->InitBoss(Wave.BossData);
        OnBossSpawned.Broadcast(Boss);   // HUD 체력바가 구독
    }
}

void UVSDifficultySubsystem::Tick(float DeltaTime)
{
    if (!CanSpawn()) return;

    ElapsedTime += DeltaTime;
    OnTimeChanged.Broadcast(ElapsedTime);

    // 클리어 판정
    if (WaveData && ElapsedTime >= WaveData->TotalRunTime)
    {
        bGameClear = true;
        OnRunCleared.Broadcast();
        return;
    }

    AdvanceWave();

    // 자동 스폰(보스·잡몹·엘리트) 전체 봉인.
    if (bWaveSpawnDisabled) return;

    TrySpawnWaveBoss();

    if (const FVSWaveEntry* Wave = GetActiveWave())
    {
        SpawnWaveEnemies(*Wave, DeltaTime);
    }
}

void UVSDifficultySubsystem::SpawnWaveEnemies(const FVSWaveEntry& Wave, float DeltaTime)
{
    AVSEnemyManager* Mgr = GetEnemyManager();
    if (!Mgr) return;

    // 일반 적 스폰
    if (Wave.EnemyType && SpawnTimer.TryConsumeInterval(DeltaTime))
    {
        for (int32 i = 0; i < Wave.SpawnPerTick; ++i)
        {
            Mgr->SpawnEnemy(Wave.EnemyType, Wave.HealthMult);
        }
    }

    // 엘리트 적 스폰
    if (Wave.EliteType && EliteTimer.TryConsumeInterval(DeltaTime))
    {
        Mgr->SpawnEnemy(Wave.EliteType, Wave.HealthMult);
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

void UVSDifficultySubsystem::RegisterPlayerCharacter(AVSPlayerCharacter* InCharacter)
{
    if (!InCharacter) return;

    PlayerCharacter = InCharacter;
    InCharacter->OnPlayerDied.AddUObject(this, &UVSDifficultySubsystem::HandlePlayerDied);
}

void UVSDifficultySubsystem::ApplyWaveTimers(const FVSWaveEntry& Wave)
{
    SpawnTimer.SetInterval(Wave.SpawnInterval);
    EliteTimer.SetInterval(Wave.EliteInterval, /*bResetAccumulator=*/true);
}