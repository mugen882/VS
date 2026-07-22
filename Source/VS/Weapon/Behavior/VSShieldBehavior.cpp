#include "Weapon/Behavior/VSShieldBehavior.h"
#include "Weapon/VSShieldArea.h"

void UVSShieldBehavior::OnAdded(UVSWeaponComponent* Comp, FVSWeaponInstance& W)
{
    SpawnShield(Comp, W);
}

void UVSShieldBehavior::OnUpgraded(UVSWeaponComponent* Comp, FVSWeaponInstance& W)
{
    if (W.ShieldActor)
        W.ShieldActor->SetRadius(W.GetShieldRadius());
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

    Weapon.ShieldActor = Comp->GetWorld()->SpawnActor<AVSShieldArea>(
        Weapon.Data->ShieldConfig.ShieldClass,
        Comp->GetFloorLocation(),
        FRotator::ZeroRotator);

    if (Weapon.ShieldActor)
    {
        Weapon.ShieldActor->SetRadius(Weapon.GetShieldRadius());
    }
}

void UVSShieldBehavior::UpdateShield(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon, float DeltaTime)
{
    AActor* Owner = Comp->GetOwner();
    if (!Owner || !Weapon.ShieldActor) return;

    Weapon.ShieldActor->SetActorLocation(Comp->GetFloorLocation());
    Comp->ApplyContinuousDamage(
        Owner->GetActorLocation(),
        Weapon.GetShieldRadius(),
        Weapon.GetDamage(),
        DeltaTime);
}