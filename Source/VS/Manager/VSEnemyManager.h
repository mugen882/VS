#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VSEnemyManager.generated.h"

class UInstancedStaticMeshComponent;
class AVSGemManager;
class UVSEnemyTypeData;

enum EEnemyCustomData
{
    Anim_StartTime = 0,
    Anim_Speed = 1,
    Anim_StartFrame = 2,
    Anim_EndFrame = 3,
    Tint_R = 4,
    Tint_G = 5,
    Tint_B = 6,
    Num = 7,
};

struct FEnemyData
{
    FVector Location;
    float   Health;

    // 타입에서 복사해온 개별 스탯 (매 틱 데이터에셋 조회 없이 순회하려고 복사)
    float   MoveSpeed = 200.f;
    float   ContactDamage = 10.f;
    int32   XPValue = 1;
    float   Scale = 1.f;
};

UCLASS()
class VS_API AVSEnemyManager : public AActor
{
    GENERATED_BODY()
public:
    AVSEnemyManager();

    // 가장 가까운 살아있는 적 찾기 (플레이어가 호출)
    int32 FindNearestEnemy(const FVector& From, float MaxRange, FVector& OutLocation) const;

    // 적에게 대미지 (투사체가 호출)
    void ApplyDamageToEnemy(int32 Index, float Damage);
    void ApplyDamageInRadius(const FVector& Center, float Radius, float Damage);

    // 지정한 타입의 적 1마리를 링 영역에 스폰 (난이도 서브시스템이 호출)
    void SpawnEnemy(const UVSEnemyTypeData* Type, float HealthMult = 1.f);

public:
    UPROPERTY(VisibleAnywhere)
    UInstancedStaticMeshComponent* ISM;

protected:
    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;

    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

protected:
    UPROPERTY(EditAnywhere)
    float MinSpawnRadius = 1500.f;  // 이 안쪽엔 스폰 안 함 (플레이어 근처 보호)

    UPROPERTY(EditAnywhere)
    float MaxSpawnRadius = 3000.f;  // 이 바깥으로도 스폰 안 함

    UPROPERTY(EditAnywhere, Category="Combat")
    float ContactRange = 100.f;      // 플레이어 타격 거리 (모든 타입 공통)

private:
    void KillEnemy(int32 Index);
    void UpdateEnemies(float DeltaTime);

private:
    TArray<FEnemyData> Enemies;

    TObjectPtr<AVSGemManager> GemManager;

    // 동시 최대 적 수 (SpawnEnemy 초입에서 검사하는 안전장치)
    UPROPERTY(EditAnywhere, Category="Spawn")
    int32 MaxEnemies = 500;
};