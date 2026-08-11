#pragma once
#include "CoreMinimal.h"
#include "VSProjectile.generated.h"

class AVSEnemyManager;

UCLASS()
class AVSProjectile : public AActor
{
    GENERATED_BODY()
public:
    AVSProjectile();

    void SetEnemyManager(AVSEnemyManager* InManager);

public:
    UPROPERTY(EditAnywhere)
    float Speed = 2000.f;

    UPROPERTY(EditAnywhere)
    float Damage = 30.f;

    UPROPERTY(EditAnywhere)
    float HitRadius = 60.f;

    UPROPERTY(EditAnywhere)
    float LifeSpanSeconds = 3.f;

    UPROPERTY(EditAnywhere)
    float MeshScale = 0.3f;

    // true면 플레이어 타격(보스/적 투사체), false면 적 타격(무기 투사체)
    UPROPERTY(EditAnywhere)
    bool bHitsPlayer = false;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    TObjectPtr<UStaticMeshComponent> MeshComp;

private:
	TWeakObjectPtr<AVSEnemyManager> EnemyManager;
};