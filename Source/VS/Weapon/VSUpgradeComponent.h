#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VSUpgradeComponent.generated.h"

class UVSUpgradeData;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VS_API UVSUpgradeComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    // 업그레이드 후보
    UPROPERTY(EditAnywhere, Category="Upgrade")
    TArray<TObjectPtr<UVSUpgradeData>> AllUpgrades;

    // 풀에서 랜덤 ChoiceCount만큼 추출
    TArray<UVSUpgradeData*> RollUpgrades();

    // 선택한 업그레이드 적용
    void ApplyUpgrade(UVSUpgradeData* Upgrade);

private:
    UPROPERTY(EditAnywhere, Category="Upgrade")
    int32 ChoiceCount = 3;
};