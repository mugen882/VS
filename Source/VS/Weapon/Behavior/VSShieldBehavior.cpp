#include "Weapon/Behavior/VSShieldBehavior.h"
#include "Component/VSWeaponComponent.h"
#include "Weapon/VSShieldAura.h"
#include "Character/VSPlayerCharacter.h"

void UVSShieldBehavior::OnAdded(UVSWeaponComponent* Comp, FVSWeaponInstance& W)
{
    SpawnShield(Comp, W);
}

void UVSShieldBehavior::OnUpgraded(UVSWeaponComponent* Comp, FVSWeaponInstance& W)
{
    if (ShieldAura)
        ShieldAura->SetRadius(GetShieldRadius(W));
}

void UVSShieldBehavior::OnRemoved(UVSWeaponComponent* Comp, FVSWeaponInstance& W)
{
    if (IsValid(ShieldAura))
    {
        ShieldAura->Destroy();
    }
    ShieldAura = nullptr;
}

void UVSShieldBehavior::Tick(UVSWeaponComponent* Comp, FVSWeaponInstance& W, float DeltaTime)
{
    UpdateShield(Comp, W, DeltaTime);
}

void UVSShieldBehavior::SpawnShield(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon)
{
    if (!Comp) return;

    AActor* Owner = Comp->GetOwner();
    if (!Owner || !Weapon.Data || !Weapon.Data->ShieldConfig.ShieldClass) return;

    ShieldAura = Comp->GetWorld()->SpawnActor<AVSShieldAura>(
        Weapon.Data->ShieldConfig.ShieldClass,
        Comp->GetFloorLocation(),
        FRotator::ZeroRotator);

    if (ShieldAura)
    {
        ShieldAura->SetRadius(GetShieldRadius(Weapon));
    }
}

void UVSShieldBehavior::UpdateShield(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon, float DeltaTime)
{
    AActor* Owner = Comp->GetOwner();
    if (!Owner || !ShieldAura) return;

    const FVSPassiveStatModifiers& Mods = Comp->GetStatMods();

    // 오라 배치 위치와 데미지 판정 중심을 동일하게 맞춘다(판정은 2D라 결과는 같지만 의도를 명확히).
    const FVector Center = Comp->GetFloorLocation();
    ShieldAura->SetActorLocation(Center);
    Comp->ApplyContinuousDamage(
        Center,
        GetShieldRadius(Weapon),
        Weapon.GetDamage(Mods),
        DeltaTime);
}

float UVSShieldBehavior::GetShieldRadius(const FVSWeaponInstance& Weapon) const
{
    if (!Weapon.Data) return 100.f;

    return FMath::Min(Weapon.Data->ShieldConfig.Radius + (Weapon.Level - 1) * ADD_SHIELD_RADIUS, Weapon.Data->ShieldConfig.MaxRadius);
}