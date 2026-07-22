#pragma once
#include "CoreMinimal.h"
#include "Weapon/Behavior/VSWeaponBehavior.h"
#include "VSDroneBehavior.generated.h"

class AVSDrone;

UCLASS()
class UVSDroneBehavior : public UVSWeaponBehavior
{
    GENERATED_BODY()
public:
    virtual void OnAdded(UVSWeaponComponent* Comp, FVSWeaponInstance& W) override;

protected:
    virtual void Tick(UVSWeaponComponent* Comp, FVSWeaponInstance& W, float DeltaTime) override;

private:
    void UpdateDrone(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon, float DeltaTime);
    AVSDrone* SpawnDrone(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon);
    void PositionDrone(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon);
    void FireFromDrone(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon, AVSDrone* Drone);
};