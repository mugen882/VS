#pragma once
#include "CoreMinimal.h"
#include "VSGemManager.generated.h"

class UInstancedStaticMeshComponent;

USTRUCT()
struct FVSGemData
{
    GENERATED_BODY()
    FVector Location = FVector::ZeroVector;
    int32   XPValue = 1;
    bool    bBeingCollected = false;  // 흡수 중 여부
};

UCLASS()
class VS_API AVSGemManager : public AActor
{
    GENERATED_BODY()

public:
    AVSGemManager();
    
    void SpawnGem(const FVector& Location, int32 XPValue = 1);

    void RemoveGem(int32 Index);

protected:
    void BeginPlay() override;
    void Tick(float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere)
    float MeshScale = 0.3f;

    UPROPERTY(EditAnywhere)
    float MagnetRange = 300.f;    // 이 안에 들면 흡수 시작

    UPROPERTY(EditAnywhere)
    float CollectRange = 80.f;    // 이 안이면 획득 완료

    UPROPERTY(EditAnywhere)
    float GemSpeed = 800.f;

private:
    void UpdateGems(float DeltaTime);

private:
    UPROPERTY(VisibleAnywhere)
    UInstancedStaticMeshComponent* GemISM;

    TArray<FVSGemData> Gems;
};