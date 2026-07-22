#pragma once
#include "CoreMinimal.h"
#include "Weapon/Behavior/VSWeaponBehavior.h"
#include "VSMultiShotBehavior.generated.h"

UCLASS()
class UVSMultiShotBehavior : public UVSWeaponBehavior
{
    GENERATED_BODY()
protected:
    virtual void Tick(UVSWeaponComponent* Comp, FVSWeaponInstance& W, float DeltaTime) override;

private:
    void FireMultiShot(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon);
};