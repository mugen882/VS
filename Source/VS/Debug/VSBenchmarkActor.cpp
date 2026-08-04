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
    // stat unit이 보여주는 것과 같은 값. 스레드별로 나뉘어 있어야
    // "CPU가 병목인가 GPU가 병목인가"를 구분할 수 있다.
    FrameMs.Add(FApp::GetDeltaTime() * 1000.f);
    GameMs.Add(FPlatformTime::ToMilliseconds(GGameThreadTime));
    RenderMs.Add(FPlatformTime::ToMilliseconds(GRenderThreadTime));
    // GGPUFrameTime 전역은 모듈 경계에서 링크가 안 잡히므로 RHI 접근자를 쓴다
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
    UE_LOG(LogVSBench, Log, TEXT(" Frame avg : %.2f ms (%.1f FPS)"), AvgFrame, 1000.f / FMath::Max(AvgFrame, KINDA_SMALL_NUMBER));
    UE_LOG(LogVSBench, Log, TEXT(" Frame p95 : %.2f ms"), P95Frame);
    UE_LOG(LogVSBench, Log, TEXT(" Frame max : %.2f ms"), MaxFrame);
    UE_LOG(LogVSBench, Log, TEXT(" Game avg  : %.2f ms"), AvgGame);
    UE_LOG(LogVSBench, Log, TEXT(" Render avg: %.2f ms"), AvgRender);
    UE_LOG(LogVSBench, Log, TEXT(" GPU avg   : %.2f ms"), AvgGPU);
    UE_LOG(LogVSBench, Log, TEXT("=============================================="));

    // 결과 누적 — 여러 번 돌린 결과를 그대로 표로 옮길 수 있다
    // 탭 구분. 스프레드시트에 붙여넣으면 셀로 바로 나뉜다
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
    // ISM 적은 액터가 아니라 배열 원소이므로 Destroy로 사라지지 않는다.
    // 정리하지 않으면 다음 실행에 이전 개체가 그대로 남아 마리 수가 누적된다.
    if (AVSEnemyManager* Mgr = GetEnemyManager())
    {
        Mgr->ClearAllEnemies();
    }

    for (AVSBenchmarkDummy* Dummy : SpawnedDummies)
    {
        if (IsValid(Dummy))
        {
            Dummy->Destroy();
        }
    }
    SpawnedDummies.Reset();

    Phase = EVSBenchPhase::Idle;
    PhaseTimer = 0.f;

    SetGameplaySpawnPaused(false);
    SetCombatEnabled(true);
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
