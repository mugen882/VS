#include "Weapon/Behavior/VSMultiShotBehavior.h"
#include "Weapon/VSProjectile.h"
#include "Character/VSPlayerCharacter.h"

void UVSMultiShotBehavior::Tick(UVSWeaponComponent* Comp, FVSWeaponInstance& W, float DeltaTime)
{
    if (!Comp) return;

    const FVSStatModifiers& Mods = Comp->GetStatMods();

    W.CooldownTimer -= DeltaTime;
    if (W.CooldownTimer <= 0.f)
    {
        FireMultiShot(Comp, W);
        W.CooldownTimer = W.GetCooldown(Mods);
    }
}

void UVSMultiShotBehavior::FireMultiShot(UVSWeaponComponent* Comp, FVSWeaponInstance& Weapon)
{
    if (!Comp) return;

    AActor* Owner = Comp->GetOwner();
    if (!Owner) return;

    const FVSStatModifiers& Mods = Comp->GetStatMods();

    const FVector OwnerLoc = Owner->GetActorLocation();
    const float Damage = Weapon.GetDamage(Mods);
    
    AVSEnemyManager* EnemyManager = Comp->GetEnemyManager();
    if (!EnemyManager) return;
    
    FVector TargetLoc;
    if (EnemyManager->FindNearestEnemy(OwnerLoc, Weapon.Data->BaseRange, TargetLoc) == INDEX_NONE)
    {
        return;
    }

    // 기준 방향: 가장 가까운 적
    FVector BaseDir = (TargetLoc - OwnerLoc).GetSafeNormal2D();

    // 부채꼴로 여러 발
    const int32 Count = Weapon.Data->ProjectilesPerShot;
    const float SpreadDeg = Weapon.Data->MultiShotConfig.SpreadAngle;   // 전체 퍼짐 각도
    const float Step = (Count > 1) ? SpreadDeg / (Count - 1) : 0.f;
    const float StartOffset = -SpreadDeg * 0.5f;

    for (int32 i = 0; i < Count; ++i)
    {
        const float AngleDeg = StartOffset + Step * i;
        const FVector Dir = BaseDir.RotateAngleAxis(AngleDeg, FVector::UpVector);
        const FRotator SpawnRot = Dir.Rotation();

        AVSProjectile* Proj = Comp->GetWorld()->SpawnActor<AVSProjectile>(
            Weapon.Data->ProjectileClass, OwnerLoc, SpawnRot);
        if (Proj)
            Proj->Damage = Damage;
    }
}