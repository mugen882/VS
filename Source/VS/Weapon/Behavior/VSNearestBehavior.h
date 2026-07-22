#pragma once
#include "CoreMinimal.h"
#include "Weapon/Behavior/VSWeaponBehavior.h"
#include "VSNearestBehavior.generated.h"

UCLASS()
class UVSNearestBehavior : public UVSWeaponBehavior
{
    GENERATED_BODY()
protected:
    virtual void Tick(UVSWeaponComponent* Comp, FVSWeaponInstance& W, float DeltaTime) override;

private:
    bool FireProjectileAtNearest(UVSWeaponComponent* Comp, const FVector& From, float Range, TSubclassOf<AVSProjectile> ProjClass, float Damage);
};