#pragma once
#include "CoreMinimal.h"
#include "Weapon/Behavior/VSWeaponBehavior.h"
#include "VSShieldBehavior.generated.h"

UCLASS()
class UVSShieldBehavior : public UVSWeaponBehavior
{
    GENERATED_BODY()
public:
    virtual void OnAdded(UVSWeaponComponent* Comp, FVSWeaponInstance& W) override;
    virtual void OnUpgraded(UVSWeaponComponent* Comp, FVSWeaponInstance& W) override;

protected:
    virtual void Tick(UVSWeaponComponent* Comp, FVSWeaponInstance& W, float DeltaTime) override;

private:
    void SpawnShield(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon);
    void UpdateShield(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon, float DeltaTime);
};