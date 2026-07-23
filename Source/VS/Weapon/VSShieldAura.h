#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VSShieldAura.generated.h"

UCLASS()
class VS_API AVSShieldAura : public AActor
{
    GENERATED_BODY()
public:
    AVSShieldAura();
    void SetRadius(float Radius);   // 반경에 맞춰 스케일 조정

protected:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> AuraMesh;   // 납작한 원판
};