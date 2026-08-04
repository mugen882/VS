#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Common/VSDefine.h"
#include "VSBenchmarkActor.generated.h"

class UVSEnemyTypeData;
class AVSEnemyManager;
class AVSBenchmarkDummy;

UENUM()
enum class EVSBenchPhase : uint8
{
    Idle,
    Warmup,     // 스폰 직후. 셰이더 컴파일·스트리밍이 끝나길 기다린다
    Sampling,   // 프레임 타임 수집
};

UENUM()
enum class EVSBenchMode : uint8
{
    ISM,        // AVSEnemyManager (ISM + AnimToTexture)
    Actors,     // AVSBenchmarkDummy (액터 + 스켈레탈 메시)
};

/**
 * ISM+AnimToTexture 경로와 액터+스켈레탈 경로의 프레임 타임을 같은 조건에서 비교한다.
 *
 * 레벨에 하나 배치하고 디테일에서 BenchEnemyType / DummyClass를 지정한 뒤,
 * 콘솔에서 실행한다:
 *
 *      VSBench 300         // ISM 경로로 300마리
 *      VSBenchActor 300    // 액터 경로로 300마리
 *      VSBenchClear        // 정리
 *
 * 결과는 출력 로그와 Saved/Benchmark/VSBenchmark.tsv 양쪽에 남는다.
 */
UCLASS()
class VS_API AVSBenchmarkActor : public AActor
{
    GENERATED_BODY()

public:
    AVSBenchmarkActor();

    virtual void Tick(float DeltaTime) override;

    // 콘솔에서 호출되는 진입점 (AVSPlayerController의 exec 함수가 전달)
    void RunBenchmark(EVSBenchMode Mode, int32 Count);
    void ClearAll();

    // --- 설정 ---

    // ISM 경로에서 스폰할 적 타입. 비교 공정성을 위해 두 경로가 같은 스탯을 쓰도록 한다
    UPROPERTY(EditAnywhere, Category = "Benchmark")
    TObjectPtr<UVSEnemyTypeData> BenchEnemyType;

    // 액터 경로에서 스폰할 더미 클래스 (메시·애님 BP가 지정된 BP)
    UPROPERTY(EditAnywhere, Category = "Benchmark")
    TSubclassOf<AVSBenchmarkDummy> DummyClass;

    // 스폰 후 이 시간만큼은 버린다. 첫 프레임의 셰이더 컴파일·힛치를 평균에서 제외
    UPROPERTY(EditAnywhere, Category = "Benchmark", meta = (ClampMin = "0"))
    float WarmupSeconds = 2.f;

    UPROPERTY(EditAnywhere, Category = "Benchmark", meta = (ClampMin = "0.5"))
    float SampleSeconds = 5.f;

    // 액터 경로 스폰 링 반경
    UPROPERTY(EditAnywhere, Category = "Benchmark")
    float ActorSpawnMinRadius = MIN_SPAWN_RADIUS;

    UPROPERTY(EditAnywhere, Category = "Benchmark")
    float ActorSpawnMaxRadius = MAX_SPAWN_RADIUS;

    // 측정 중 플레이어 무기를 멈춘다.
    // 켜두면 ISM 쪽만 사격·피격·사망·젬 드롭 비용을 추가로 떠안아 비교가 성립하지 않는다.
    UPROPERTY(EditAnywhere, Category = "Benchmark")
    bool bDisableCombat = true;

private:
    // EnemyManager는 게임모드가 런타임에 스폰하므로 BeginPlay 시점엔 없을 수 있다.
    // 생성 순서에 기대지 않고 필요할 때 찾아 캐시한다.
    AVSEnemyManager* GetEnemyManager();

    void SpawnISM(int32 Count);
    void SpawnActors(int32 Count);
    void CollectSample();
    void Report();

    // 정규 웨이브 스폰을 멈춰 측정 중 개체 수가 늘지 않게 한다
    void SetGameplaySpawnPaused(bool bPaused);
    void SetCombatEnabled(bool bEnabled);

    static float Percentile(TArray<float>& Sorted, float Fraction);

private:
    EVSBenchPhase Phase = EVSBenchPhase::Idle;
    EVSBenchMode CurrentMode = EVSBenchMode::ISM;
    int32 RequestedCount = 0;
    float PhaseTimer = 0.f;

    TArray<float> FrameMs;
    TArray<float> GameMs;
    TArray<float> RenderMs;
    TArray<float> GPUMs;

    UPROPERTY()
    TArray<TObjectPtr<AVSBenchmarkDummy>> SpawnedDummies;

    UPROPERTY()
    TObjectPtr<AVSEnemyManager> EnemyManager;
};
