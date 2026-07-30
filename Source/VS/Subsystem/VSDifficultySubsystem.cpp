#include "Subsystem/VSDifficultySubsystem.h"
#include "Manager/VSEnemyManager.h"
#include "Data/VSWaveData.h"
#include "Enemy/VSBossEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Character/VSPlayerCharacter.h"

void UVSDifficultySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    ElapsedTime = 0.f;
    SpawnAccumulator = 0.f;
    EliteAccumulator = 0.f;
    CurrentWaveIndex = 0;
    KillCount = 0;
    bGameOver = false;
    bUpgradeSelecting = false;
    bGameClear = false;
}

void UVSDifficultySubsystem::SetWaveData(UVSWaveData* InWaveData)
{
    WaveData = InWaveData;

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

const FVSWaveEntry* UVSDifficultySubsystem::ResolveCurrentWave()
{
    if (!WaveData || WaveData->Waves.Num() == 0)
        return nullptr;

    // 경과 시간이 다음 웨이브의 시작 시각을 넘으면 전환
    while (CurrentWaveIndex + 1 < WaveData->Waves.Num()
        && ElapsedTime >= WaveData->Waves[CurrentWaveIndex + 1].StartTime)
    {
        ++CurrentWaveIndex;
        EliteAccumulator = 0.f;
    }

    // 현재 웨이브가 이전에 보스 스폰한 웨이브와 다르면 스폰
    if (CurrentWaveIndex != LastBossWaveIndex)
    {
        LastBossWaveIndex = CurrentWaveIndex;
        SpawnWaveBoss(WaveData->Waves[CurrentWaveIndex]);
    }

    // 아직 첫 웨이브 StartTime 전이면 스폰 안 함
    if (ElapsedTime < WaveData->Waves[CurrentWaveIndex].StartTime)
        return nullptr;

    return &WaveData->Waves[CurrentWaveIndex];
}

void UVSDifficultySubsystem::SpawnWaveBoss(const FVSWaveEntry& Wave)
{
    if (!Wave.BossClass) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // 플레이어 주변 링 영역에 스폰
    FVector SpawnLoc = FVector::ZeroVector;
    if (APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0))
    {
        const float Angle = FMath::FRandRange(0.f, 2.f * PI);
        const float Dist = BOSS_SPAWN_DIST;
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

    AVSEnemyManager* Mgr = GetEnemyManager();
    if (!Mgr) return;

    const FVSWaveEntry* Wave = ResolveCurrentWave();
    if (!Wave) return;

    // 일반 적 스폰
    if (Wave->EnemyType && Wave->SpawnInterval > 0.f && Wave->SpawnInterval != 0)
    {
        SpawnAccumulator += DeltaTime;
        if (SpawnAccumulator >= Wave->SpawnInterval)
        {
            SpawnAccumulator -= Wave->SpawnInterval;
            for (int32 i = 0; i < Wave->SpawnPerTick; ++i)
                Mgr->SpawnEnemy(Wave->EnemyType, Wave->HealthMult);
        }
    }

    // 엘리트 적 스폰
    if (Wave->EliteType && Wave->EliteInterval > 0.f && Wave->EliteInterval != 0)
    {
        EliteAccumulator += DeltaTime;
        if (EliteAccumulator >= Wave->EliteInterval)
        {
            EliteAccumulator -= Wave->EliteInterval;
            Mgr->SpawnEnemy(Wave->EliteType, Wave->HealthMult);
        }
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