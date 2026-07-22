#pragma once
#include "CoreMinimal.h"
#include "Weapon/Behavior/VSWeaponBehavior.h"
#include "VSOrbitBehavior.generated.h"

UCLASS()
class UVSOrbitBehavior : public UVSWeaponBehavior
{
    GENERATED_BODY()
public:
    UVSOrbitBehavior();

    virtual void OnAdded(UVSWeaponComponent* Comp, FVSWeaponInstance& W) override;
    virtual void OnUpgraded(UVSWeaponComponent* Comp, FVSWeaponInstance& W) override;

protected:
    virtual void Tick(UVSWeaponComponent* Comp, FVSWeaponInstance& W, float DeltaTime) override;

private:
    AVSOrbitProjectile* SpawnSingleBall(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon);
    void PositionBalls(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon);
    void CheckHits(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon, float DeltaTime);
};