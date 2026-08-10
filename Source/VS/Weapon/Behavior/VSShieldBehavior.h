#pragma once
#include "CoreMinimal.h"
#include "Weapon/Behavior/VSWeaponBehavior.h"
#include "VSShieldBehavior.generated.h"

class AVSShieldAura;

UCLASS()
class VS_API UVSShieldBehavior : public UVSWeaponBehavior
{
    GENERATED_BODY()
public:
    virtual void OnAdded(UVSWeaponComponent* Comp, FVSWeaponInstance& W) override;
    virtual void OnUpgraded(UVSWeaponComponent* Comp, FVSWeaponInstance& W) override;
    virtual void OnRemoved(UVSWeaponComponent* Comp, FVSWeaponInstance& W) override;

protected:
    virtual void Tick(UVSWeaponComponent* Comp, FVSWeaponInstance& W, float DeltaTime) override;

private:
    void SpawnShield(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon);
    void UpdateShield(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon, float DeltaTime);

    // 레벨/데이터로 계산하는 실드 반경 (실드 전용)
    float GetShieldRadius(const FVSWeaponInstance& Weapon) const;

protected:
    UPROPERTY(Transient)
    TObjectPtr<AVSShieldAura> ShieldAura;
};