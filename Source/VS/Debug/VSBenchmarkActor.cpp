#include "Debug/VSBenchmarkActor.h"
#include "Debug/VSBenchmarkDummy.h"
#include "Manager/VSEnemyManager.h"
#include "Subsystem/VSDifficultySubsystem.h"
#include "Data/VSEnemyTypeData.h"
#include "Kismet/GameplayStatics.h"
#include "Character/VSPlayerCharacter.h"
#include "Component/VSWeaponComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "RenderCore.h"
#include "RHI.h"

DEFINE_LOG_CATEGORY_STATIC(LogVSBench, Log, All);

AVSBenchmarkActor::AVSBenchmarkActor()
{
    PrimaryActorTick.bCanEverTick = true;
    // 측정 대상이 아니므로 다른 액터보다 먼저 돌아 샘플 시점을 일정하게 유지한다
    PrimaryActorTick.TickGroup = TG_PrePhysics;
}

AVSEnemyManager* AVSBenchmarkActor::GetEnemyManager()
{
    if (!EnemyManager)
    {
        EnemyManager = Cast<AVSEnemyManager>(
            UGameplayStatics::GetActorOfClass(this, AVSEnemyManager::StaticClass()));
    }
    return EnemyManager;
}

void AVSBenchmarkActor::RunBenchmark(EVSBenchMode Mode, int32 Count)
{
    if (Phase != EVSBenchPhase::Idle)
    {
        UE_LOG(LogVSBench, Warning, TEXT("이미 측정 중입니다. 끝난 뒤 다시 실행하세요."));
        return;
    }
    if (Count <= 0) return;

    ClearAll();

    CurrentMode = Mode;
    RequestedCount = Count;

    SetGameplaySpawnPaused(true);
    if (bDisableCombat) SetCombatEnabled(false);

    if (Mode == EVSBenchMode::ISM)
    {
        SpawnISM(Count);
    }
    else
    {
        SpawnActors(Count);
    }

    Phase = EVSBenchPhase::Warmup;
    PhaseTimer = 0.f;

    UE_LOG(LogVSBench, Log, TEXT("[Bench] %s %d마리 스폰. %.1f초 워밍업 후 %.1f초 측정."),
        Mode == EVSBenchMode::ISM ? TEXT("ISM") : TEXT("Actor"),
        Count, WarmupSeconds, SampleSeconds);
}

void AVSBenchmarkActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (Phase == EVSBenchPhase::Idle) return;

    PhaseTimer += DeltaTime;

    if (Phase == EVSBenchPhase::Warmup)
    {
        if (PhaseTimer >= WarmupSeconds)
        {
            Phase = EVSBenchPhase::Sampling;
            PhaseTimer = 0.f;

            FrameMs.Reset();
            GameMs.Reset();
            RenderMs.Reset();
            GPUMs.Reset();

            UE_LOG(LogVSBench, Log, TEXT("[Bench] 측정 시작."));
        }
        return;
    }

    CollectSample();

    if (PhaseTimer >= SampleSeconds)
    {
        Report();
        Phase = EVSBenchPhase::Idle;
    }
}

void AVSBenchmarkActor::CollectSample()
{
    FrameMs.Add(FApp::GetDeltaTime() * 1000.f);
    
    // 세 값 모두 밀리초가 아니라 CPU 사이클 카운트로 들어온다.
    // GPU 시간도 RHI가 CPU 타임베이스로 환산해 저장하므로 같은 함수로 변환된다.
    // GPU만 전역 대신 접근자를 쓰는 건 멀티 GPU 인덱스를 받는 형태이기 때문.
    GameMs.Add(FPlatformTime::ToMilliseconds(GGameThreadTime));
    RenderMs.Add(FPlatformTime::ToMilliseconds(GRenderThreadTime));
    GPUMs.Add(FPlatformTime::ToMilliseconds(RHIGetGPUFrameCycles()));
}

float AVSBenchmarkActor::Percentile(TArray<float>& Sorted, float Fraction)
{
    if (Sorted.Num() == 0) return 0.f;
    const int32 Idx = FMath::Clamp(
        FMath::RoundToInt(Fraction * (Sorted.Num() - 1)), 0, Sorted.Num() - 1);
    return Sorted[Idx];
}

void AVSBenchmarkActor::Report()
{
    if (FrameMs.Num() == 0)
    {
        UE_LOG(LogVSBench, Warning, TEXT("[Bench] 샘플이 없습니다."));
        SetGameplaySpawnPaused(false);
        SetCombatEnabled(true);
        return;
    }

    auto Avg = [](const TArray<float>& A)
    {
        float Sum = 0.f;
        for (float V : A) Sum += V;
        return Sum / A.Num();
    };

    TArray<float> SortedFrame = FrameMs;
    SortedFrame.Sort();

    const float AvgFrame = Avg(FrameMs);
    const float P95Frame = Percentile(SortedFrame, 0.95f);
    const float MaxFrame = SortedFrame.Last();
    const float AvgGame = Avg(GameMs);
    const float AvgRender = Avg(RenderMs);
    const float AvgGPU = Avg(GPUMs);

    AVSEnemyManager* Mgr = GetEnemyManager();
    const int32 ActualCount = (CurrentMode == EVSBenchMode::ISM)
        ? (Mgr ? Mgr->GetEnemyCount() : 0)
        : SpawnedDummies.Num();

    const FString ModeName = (CurrentMode == EVSBenchMode::ISM) ? TEXT("ISM+AnimToTexture") : TEXT("Actor+Skeletal");

    UE_LOG(LogVSBench, Log, TEXT("================ VS Benchmark ================"));
    UE_LOG(LogVSBench, Log, TEXT(" Mode      : %s"), *ModeName);
    UE_LOG(LogVSBench, Log, TEXT(" Count     : %d (요청 %d)"), ActualCount, RequestedCount);
    UE_LOG(LogVSBench, Log, TEXT(" Samples   : %d"), FrameMs.Num());
    UE_LOG(LogVSBench, Log, TEXT(" Frame avg : %.2f ms (%.1f FPS)"), AvgFrame, 1000.f / FMath::Max(AvgFrame, UE_KINDA_SMALL_NUMBER));
    UE_LOG(LogVSBench, Log, TEXT(" Frame p95 : %.2f ms"), P95Frame);
    UE_LOG(LogVSBench, Log, TEXT(" Frame max : %.2f ms"), MaxFrame);
    UE_LOG(LogVSBench, Log, TEXT(" Game avg  : %.2f ms"), AvgGame);
    UE_LOG(LogVSBench, Log, TEXT(" Render avg: %.2f ms"), AvgRender);
    UE_LOG(LogVSBench, Log, TEXT(" GPU avg   : %.2f ms"), AvgGPU);
    UE_LOG(LogVSBench, Log, TEXT("=============================================="));

    // 결과 누적 — 여러 번 돌린 결과를 그대로 표로 옮길 수 있다
    const FString CsvPath = FPaths::ProjectSavedDir() / TEXT("Benchmark/VSBenchmark.tsv");
    if (!FPaths::FileExists(CsvPath))
    {
        FFileHelper::SaveStringToFile(
            FString(TEXT("Timestamp\tMode\tCount\tSamples\tFrameAvgMs\tFrameP95Ms\tFrameMaxMs\tGameAvgMs\tRenderAvgMs\tGPUAvgMs\n")),
            *CsvPath);
    }

    const FString Row = FString::Printf(TEXT("%s\t%s\t%d\t%d\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\t%.3f\n"),
        *FDateTime::Now().ToString(), *ModeName, ActualCount, FrameMs.Num(),
        AvgFrame, P95Frame, MaxFrame, AvgGame, AvgRender, AvgGPU);

    FFileHelper::SaveStringToFile(Row, *CsvPath,
        FFileHelper::EEncodingOptions::AutoDetect,
        &IFileManager::Get(), EFileWrite::FILEWRITE_Append);

    UE_LOG(LogVSBench, Log, TEXT("[Bench] 결과 기록: %s"), *CsvPath);

    SetGameplaySpawnPaused(false);
    SetCombatEnabled(true);

    UE_LOG(LogVSBench, Warning, TEXT("[Bench] tsv Saved."));
}

void AVSBenchmarkActor::SpawnISM(int32 Count)
{
    AVSEnemyManager* Mgr = GetEnemyManager();
    if (!Mgr)
    {
        UE_LOG(LogVSBench, Error, TEXT("[Bench] AVSEnemyManager를 찾지 못했습니다. 게임모드의 EnemyManagerClass를 확인하세요."));
        return;
    }
    if (!BenchEnemyType)
    {
        UE_LOG(LogVSBench, Error, TEXT("[Bench] BenchEnemyType이 지정되지 않았습니다."));
        return;
    }

    for (int32 i = 0; i < Count; ++i)
    {
        Mgr->SpawnEnemy(BenchEnemyType, 1.f, nullptr);
    }
}

void AVSBenchmarkActor::SpawnActors(int32 Count)
{
    UWorld* World = GetWorld();
    if (!World || !DummyClass)
    {
        UE_LOG(LogVSBench, Error, TEXT("[Bench] DummyClass가 지정되지 않았습니다."));
        return;
    }

    const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    const FVector Center = Player ? Player->GetActorLocation() : GetActorLocation();

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (int32 i = 0; i < Count; ++i)
    {
        // ISM 경로와 같은 링 분포로 뿌린다
        const float Angle = FMath::FRandRange(0.f, 2.f * PI);
        const float Radius = FMath::FRandRange(ActorSpawnMinRadius, ActorSpawnMaxRadius);
        const FVector Loc(
            Center.X + FMath::Cos(Angle) * Radius,
            Center.Y + FMath::Sin(Angle) * Radius,
            0.f);

        if (AVSBenchmarkDummy* Dummy = World->SpawnActor<AVSBenchmarkDummy>(
                DummyClass, Loc, FRotator::ZeroRotator, Params))
        {
            SpawnedDummies.Add(Dummy);
        }
    }
}

void AVSBenchmarkActor::ClearAll()
{
    if (AVSEnemyManager* Mgr = GetEnemyManager())
    {
        Mgr->ClearAllEnemies();
    }

    ClearDummies();

    Phase = EVSBenchPhase::Idle;
    PhaseTimer = 0.f;

    SetGameplaySpawnPaused(false);
    SetCombatEnabled(true);
}

void AVSBenchmarkActor::ClearDummies()
{
    for (AVSBenchmarkDummy* Dummy : SpawnedDummies)
    {
        if (IsValid(Dummy))
        {
            Dummy->Destroy();
        }
    }
    SpawnedDummies.Reset();
}

void AVSBenchmarkActor::SetGameplaySpawnPaused(bool bPaused)
{
    if (UWorld* World = GetWorld())
    {
        if (UVSDifficultySubsystem* Diff = World->GetSubsystem<UVSDifficultySubsystem>())
        {
            Diff->SetBenchmarkPaused(bPaused);
        }
    }
}

void AVSBenchmarkActor::SetCombatEnabled(bool bEnabled)
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    AVSPlayerCharacter* PC = Cast<AVSPlayerCharacter>(Player);
    if (!PC) return;

    if (UVSWeaponComponent* Weapon = PC->GetWeaponComponent())
    {
        Weapon->SetComponentTickEnabled(bEnabled);
    }
}
