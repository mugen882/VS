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
    Scene,      // 아무것도 스폰하지 않고 현재 화면을 그대로 측정 (렌더러 설정 A/B용)
};

UCLASS()
class VS_API AVSBenchmarkActor : public AActor
{
    GENERATED_BODY()

public:
    AVSBenchmarkActor();

    virtual void Tick(float DeltaTime) override;

    // 콘솔에서 호출되는 진입점 (AVSPlayerController의 exec 함수가 전달)
    void RunBenchmark(EVSBenchMode Mode, int32 Count);

    /**
     * 현재 화면을 그대로 측정한다. 스폰도 정리도 하지 않는다.
     * 렌더러 CVar를 바꿔가며 같은 장면을 비교할 때 쓴다.
     * WarmupSec/SampleSec에 0 이하를 주면 액터의 기본값을 쓴다.
     */
    void RunSceneBenchmark(float WarmupSec, float SampleSec);

    void ClearAll();

    // 액터 경로에서 스폰한 더미 액터만 제거 (ISM 적은 건드리지 않음)
    void ClearDummies();

    // --- 설정 ---

    // ISM 경로에서 스폰할 적 타입.
    UPROPERTY(EditAnywhere, Category = "Benchmark")
    TObjectPtr<UVSEnemyTypeData> BenchEnemyType;

    // 액터 경로에서 스폰할 더미 클래스
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
    UPROPERTY(EditAnywhere, Category = "Benchmark")
    bool bDisableCombat = true;

private:
    // 필요할 때 찾아 캐시한다.
    AVSEnemyManager* GetEnemyManager();

    void SpawnISM(int32 Count);
    void SpawnActors(int32 Count);
    void CollectSample();
    void Report();

    // 정규 웨이브 스폰을 멈춰 측정 중 개체 수가 늘지 않게 한다
    void SetGameplaySpawnPaused(bool bPaused);
    void SetCombatEnabled(bool bEnabled);

    static float Percentile(const TArray<float>& Sorted, float Fraction);

    void BeginSampling();

    // 워밍업 진입 공통부 (스폰 정지·전투 정지·타이머 초기화)
    void EnterWarmup();

    // 결과 행에 남길 렌더러 설정 문자열. 어떤 설정으로 잰 수치인지 나중에 알 수 있게 한다
    static FString DescribeRenderSettings();

private:
    EVSBenchPhase Phase = EVSBenchPhase::Idle;
    EVSBenchMode CurrentMode = EVSBenchMode::ISM;
    int32 RequestedCount = 0;
    float PhaseTimer = 0.f;

    // 이번 측정에 실제로 쓸 시간. RunSceneBenchmark가 인자로 덮어쓸 수 있다
    float ActiveWarmupSeconds = 0.f;
    float ActiveSampleSeconds = 0.f;

    TArray<float> FrameMs;
    TArray<float> GameMs;
    TArray<float> RenderMs;
    TArray<float> GPUMs;

    UPROPERTY()
    TArray<TObjectPtr<AVSBenchmarkDummy>> SpawnedDummies;

    UPROPERTY()
    TObjectPtr<AVSEnemyManager> EnemyManager;
};
