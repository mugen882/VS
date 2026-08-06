#include "Weapon/Behavior/VSShieldBehavior.h"
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

    ShieldAura->SetActorLocation(Comp->GetFloorLocation());
    Comp->ApplyContinuousDamage(
        Owner->GetActorLocation(),
        GetShieldRadius(Weapon),
        Weapon.GetDamage(Mods),
        DeltaTime);
}

float UVSShieldBehavior::GetShieldRadius(const FVSWeaponInstance& Weapon) const
{
    if (!Weapon.Data) return 100.f;

    return FMath::Min(Weapon.Data->ShieldConfig.Radius + (Weapon.Level - 1) * ADD_SHIELD_RADIUS, Weapon.Data->ShieldConfig.MaxRadius);
}