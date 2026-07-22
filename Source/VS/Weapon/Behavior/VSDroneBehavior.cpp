#include "Weapon/Behavior/VSDroneBehavior.h"
#include "Weapon/VSDrone.h"
#include "Weapon/VSProjectile.h"
#include "Character/VSCharacter.h"

void UVSDroneBehavior::OnAdded(UVSWeaponComponent* Comp, FVSWeaponInstance& W)
{
    SpawnDrone(Comp, W);
}

void UVSDroneBehavior::Tick(UVSWeaponComponent* Comp, FVSWeaponInstance& W, float DeltaTime)
{
    UpdateDrone(Comp, W, DeltaTime);
}

void UVSDroneBehavior::UpdateDrone(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon, float DeltaTime)
{
    if (!Weapon.Drone || !Comp) return;

    const FVSStatModifiers& Mods = Comp->GetStatMods();
    
    PositionDrone(Comp, Weapon);
    Weapon.CooldownTimer -= DeltaTime;
    if (Weapon.CooldownTimer <= 0.f)
    {
        FireFromDrone(Comp, Weapon, Weapon.Drone);
        Weapon.CooldownTimer = Weapon.GetCooldown(Mods);
    }
}

AVSDrone* UVSDroneBehavior::SpawnDrone(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon)
{
    if (!Comp) return nullptr;

    AActor* Owner = Comp->GetOwner();
    if (!Owner || !Weapon.Data || !Weapon.Data->DroneConfig.DroneClass) return nullptr;

    AVSDrone* Drone = Comp->GetWorld()->SpawnActor<AVSDrone>(
        Weapon.Data->DroneConfig.DroneClass,
        Owner->GetActorLocation(), FRotator::ZeroRotator);
    if (Drone)
    {
        Drone->SetActorScale3D(FVector(Weapon.Data->DroneConfig.Scale));
        Weapon.Drone = Drone;
    }
    return Drone;
}

void UVSDroneBehavior::PositionDrone(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon)
{
    if (!Comp) return;

    AActor* Owner = Comp->GetOwner();

    if (!Owner || !Weapon.Drone) return;

    const FVector Center = Owner->GetActorLocation();
    const FVector Offset(0.f, Weapon.Data->DroneConfig.Offset, 150.f);  // 플레이어 우측 상단
    Weapon.Drone->SetActorLocation(Center + Offset);
}

void UVSDroneBehavior::FireFromDrone(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon, AVSDrone* Drone)
{
    if (!Comp) return;

    AVSEnemyManager* EnemyManager = Comp->GetEnemyManager();
    if (!EnemyManager) return;

    const FVector DroneLoc = Drone->GetActorLocation();

    FVector TargetLoc;
    if (EnemyManager->FindNearestEnemy(DroneLoc, Weapon.Data->BaseRange, TargetLoc) == INDEX_NONE)
        return;

    const FVSStatModifiers& Mods = Comp->GetStatMods();

    const FVector Dir = (TargetLoc - DroneLoc).GetSafeNormal2D();
    const float Damage = Weapon.GetDamage(Mods);
    const int32 Count = Weapon.GetProjectileCount();

    // 발사 방향에 수직인 벡터 (옆으로 벌리기용)
    const FVector Side = FVector::CrossProduct(Dir, FVector::UpVector).GetSafeNormal();
    const float Spacing = 40.f;   // 총알 간 간격
    const float StartOffset = -Spacing * (Count - 1) * 0.5f;   // 가운데 정렬

    for (int32 i = 0; i < Count; ++i)
    {
        // 같은 방향(Dir), 위치만 옆으로 벌림
        const FVector SpawnLoc = DroneLoc + Side * (StartOffset + Spacing * i);

        AVSProjectile* Proj = Comp->GetWorld()->SpawnActor<AVSProjectile>(
            Weapon.Data->ProjectileClass, SpawnLoc, Dir.Rotation());
        if (Proj)
            Proj->Damage = Damage;
    }
}