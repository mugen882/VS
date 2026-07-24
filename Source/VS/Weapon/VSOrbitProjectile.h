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
    float HitRadius = 5.f;

	UPROPERTY(EditAnywhere, Category = "Orbit")
    float Scale = 0.3f;

protected:
	void BeginPlay() override;

protected:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;
};