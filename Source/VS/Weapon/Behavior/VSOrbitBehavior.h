#pragma once
#include "CoreMinimal.h"
#include "Weapon/Behavior/VSWeaponBehavior.h"
#include "Weapon/VSOrbitProjectile.h"
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

private:
    // 이 무기 인스턴스 전용 상태
    float OrbitAngle = 0.f;   // 현재 회전 각도

    UPROPERTY(Transient)
    TArray<TObjectPtr<AVSOrbitProjectile>> OrbitBalls;
};