#pragma once
#include "CoreMinimal.h"
#include "VSProjectile.generated.h"

UCLASS()
class AVSProjectile : public AActor
{
    GENERATED_BODY()
public:
    AVSProjectile();

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

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
    TObjectPtr<UStaticMeshComponent> MeshComp;
};