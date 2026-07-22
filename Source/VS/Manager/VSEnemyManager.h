#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VSEnemyManager.generated.h"

class UInstancedStaticMeshComponent;
class AVSGemManager;

struct FEnemyData
{
    FVector Location;
    float   Health;
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

public:
    UPROPERTY(VisibleAnywhere)
    UInstancedStaticMeshComponent* ISM;

    UPROPERTY(EditAnywhere)
    int32 InstanceCount = 1000;

    UPROPERTY(EditAnywhere)
    float SpawnRadius = 2000.f;

    UPROPERTY(EditAnywhere)
    float EnemyMaxHealth = 30.f;

protected:
    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY(EditAnywhere)
    float MoveSpeed = 200.f;

    UPROPERTY(EditAnywhere)
    float MinSpawnRadius = 1500.f;  // 이 안쪽엔 스폰 안 함 (플레이어 근처 보호)

    UPROPERTY(EditAnywhere)
    float MaxSpawnRadius = 3000.f;  // 이 바깥으로도 스폰 안 함

    UPROPERTY(EditAnywhere)
    int32 GemPerXP = 1;

    UPROPERTY(EditAnywhere, Category="Combat")
    float ContactRange = 100.f;      // 플레이어 타격 거리

    UPROPERTY(EditAnywhere, Category="Combat")
    float ContactDamage = 10.f;      // 초당 대미지

private:
    void KillEnemy(int32 Index);
    void UpdateEnemies(float DeltaTime);

private:
    TArray<FEnemyData> Enemies;

    TObjectPtr<AVSGemManager> GemManager;
};