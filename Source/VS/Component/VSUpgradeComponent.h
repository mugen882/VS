#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Character/VSPassiveStatModifiers.h"
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
    // 레벨 상한과는 별개로, 더 찍어도 실제 수치가 변하지 않는 패시브인지 판정한다.
    // 예: 보유 무기가 전부 MIN_COOLDOWN_TIME에 걸린 상태의 GlobalCooldown.
    bool IsPassiveSaturated(EVSPassiveStatType StatType) const;

    UPROPERTY(EditAnywhere, Category="Upgrade")
    int32 ChoiceCount = 3;
};