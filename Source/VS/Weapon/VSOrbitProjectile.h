#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VSOrbitProjectile.generated.h"

UCLASS()
class AVSOrbitProjectile : public AActor
{
    GENERATED_BODY()
public:
    AVSOrbitProjectile();
    float Damage = 10.f;
    float HitRadius = 5.f;

	UPROPERTY(EditAnywhere, Category = "Orbit")
    float Scale = 0.3f;

    // 매 틱 위치는 WeaponComponent가 설정, 얘는 타격만
    void CheckHit();   // 근처 적에게 대미지

protected:
	void BeginPlay() override;

protected:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;
};