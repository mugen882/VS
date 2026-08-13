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

    // 모든 젬(XP 픽업)을 즉시 제거
    void ClearAllGems();

    void SetMagnetRangeMult(float Mult);   // 패시브 배율 반영

protected:
    void BeginPlay() override;
    void Tick(float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere)
    float MeshScale = 0.3f;

    UPROPERTY(EditAnywhere)
    float BaseMagnetRange = 300.f;

    UPROPERTY(EditAnywhere)
    float CollectRange = 80.f;    // 이 안이면 획득 완료

    UPROPERTY(EditAnywhere)
    float GemSpeed = 1200.f;

    // 동시에 존재할 수 있는 젬 상한. 초과 시 가장 오래된 젬부터 제거한다.
    UPROPERTY(EditAnywhere, meta=(ClampMin="1"))
    int32 MaxGems = 3000;

private:
    void UpdateGems(float DeltaTime);

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UInstancedStaticMeshComponent> GemISM;

    TArray<FVSGemData> Gems;

    float MagnetRange = 300.f;  // 실제 사용값 = BaseMagnetRange × Mult
};