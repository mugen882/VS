#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VSUpgradeComponent.generated.h"

class UVSUpgradeData;

UCLASS(ClassGroup=(VS), meta=(BlueprintSpawnableComponent))
class VS_API UVSUpgradeComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    // 업그레이드 후보
    UPROPERTY(EditAnywhere, Category="Upgrade")
    TArray<TObjectPtr<UVSUpgradeData>> AllUpgrades;

    // 풀에서 랜덤으로 ChoiceCount만큼 추출
    TArray<UVSUpgradeData*> RollUpgrades();

    // 선택한 업그레이드 적용
    void ApplyUpgrade(UVSUpgradeData* Upgrade);

    // [Cheat] AllUpgrades에 등록된 NewWeapon을 전부 획득한 뒤,
    // 보유 중인 모든 무기를 TargetLevel까지 강화한다. 카드 UI를 거치지 않는다.
    void CheatGiveAllWeapons(int32 TargetLevel);

    // [Cheat] AllUpgrades에 등록된 Passive의 StatType을 전부 TargetLevel까지 올린다.
    // 카드 UI를 거치지 않는다.
    void CheatGiveAllPassives(int32 TargetLevel);

private:
    UPROPERTY(EditAnywhere, Category="Upgrade")
    int32 ChoiceCount = 3;
};