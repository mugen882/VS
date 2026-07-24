#include "Weapon/Behavior/VSNearestBehavior.h"
#include "Weapon/VSProjectile.h"
#include "Character/VSPlayerCharacter.h"

void UVSNearestBehavior::Tick(UVSWeaponComponent* Comp, FVSWeaponInstance& W, float DeltaTime)
{
    if (!Comp) return;

    const FVSStatModifiers& Mods = Comp->GetStatMods();

    W.CooldownTimer -= DeltaTime;
    if (W.CooldownTimer <= 0.f)
    {
        AActor* Owner = Comp->GetOwner();
        if (Owner)
        {
            FireProjectileAtNearest(
                Comp,
                Owner->GetActorLocation(),
                W.Data->BaseRange,
                W.Data->ProjectileClass,
                W.GetDamage(Mods));
        }
        W.CooldownTimer = W.GetCooldown(Mods);
    }
}

bool UVSNearestBehavior::FireProjectileAtNearest(UVSWeaponComponent* Comp, const FVector& From, float Range, TSubclassOf<AVSProjectile> ProjClass, float Damage)
{
    if (!Comp) return false;

    AVSEnemyManager* EnemyManager = Comp->GetEnemyManager();
    if (!EnemyManager) return false;

    FVector TargetLoc;
    if (EnemyManager->FindNearestEnemy(From, Range, TargetLoc) == INDEX_NONE)
        return false;

    const FVector Dir = (TargetLoc - From).GetSafeNormal2D();
    if (AVSProjectile* Proj = Comp->GetWorld()->SpawnActor<AVSProjectile>(ProjClass, From, Dir.Rotation()))
        Proj->Damage = Damage;

    return true;
}