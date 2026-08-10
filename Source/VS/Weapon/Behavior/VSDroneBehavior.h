#pragma once
#include "CoreMinimal.h"
#include "Weapon/Behavior/VSWeaponBehavior.h"
#include "VSDroneBehavior.generated.h"

class AVSDrone;

UCLASS()
class VS_API UVSDroneBehavior : public UVSWeaponBehavior
{
    GENERATED_BODY()
public:
    virtual void OnAdded(UVSWeaponComponent* Comp, FVSWeaponInstance& W) override;
    virtual void OnRemoved(UVSWeaponComponent* Comp, FVSWeaponInstance& W) override;

protected:
    virtual void Tick(UVSWeaponComponent* Comp, FVSWeaponInstance& W, float DeltaTime) override;

private:
    void UpdateDrone(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon, float DeltaTime);
    AVSDrone* SpawnDrone(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon);
    void PositionDrone(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon);
    void FireFromDrone(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon, AVSDrone* InDrone);

    // 레벨/데이터로 계산하는 발사체 수 (드론 전용)
    int32 GetProjectileCount(const FVSWeaponInstance& Weapon) const;

protected:
	UPROPERTY(Transient)
	TObjectPtr<AVSDrone> Drone;
};