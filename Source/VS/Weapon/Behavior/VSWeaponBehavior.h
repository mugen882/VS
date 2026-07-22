#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Component/VSWeaponComponent.h"
#include "VSWeaponBehavior.generated.h"

class UVSWeaponComponent;

UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class VS_API UVSWeaponBehavior : public UObject
{
    GENERATED_BODY()
public:
    // 무기가 추가될 때 (스폰 등)
    virtual void OnAdded(UVSWeaponComponent* Comp, FVSWeaponInstance& W) {}
    // 레벨업 할 때
    virtual void OnUpgraded(UVSWeaponComponent* Comp, FVSWeaponInstance& W) {}
    // 매 틱
    virtual void Tick(UVSWeaponComponent* Comp, FVSWeaponInstance& W, float DeltaTime) {}
};