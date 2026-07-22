#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VSShieldArea.generated.h"

UCLASS()
class VS_API AVSShieldArea : public AActor
{
    GENERATED_BODY()
public:
    AVSShieldArea();
    void SetRadius(float Radius);   // 반경에 맞춰 스케일 조정

protected:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> AreaMesh;   // 납작한 원판
};