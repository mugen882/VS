#include "Enemy/VSBossRanger.h"
#include "Data/VSBossData.h"
#include "Weapon/VSProjectile.h"

void AVSBossRanger::MoveTowardPlayer(float DeltaTime)
{
    const FVSBossPlayerInfo Info = QueryPlayer();
    if (!Info.IsValid()) return;

    ApplyKiting(Info, DeltaTime);                          // 거리 유지
    FaceDirection(Info.Dir, GetRotateSpeedDeg(), DeltaTime);   // 항상 플레이어를 바라봄
    ApplyContactDamage(Info, DeltaTime);
}

void AVSBossRanger::UpdateAttack(float DeltaTime)
{
    UVSBossRangerData* CfgData = Cast<UVSBossRangerData>(GetData());
    if (!CfgData || !CfgData->ProjectileClass) return;

    FireTimer -= DeltaTime;
    if (FireTimer <= 0.f)
    {
        FireRadialBurst();
        FireTimer = CfgData->FireInterval;
    }
}

void AVSBossRanger::FireRadialBurst()
{
    UVSBossRangerData* CfgData = Cast<UVSBossRangerData>(GetData());
    if (!CfgData) return;

    UWorld* World = GetWorld();
    if (!World || CfgData->ProjectileCount <= 0) return;

    const FVector SpawnLoc = GetActorLocation();
    const float AngleStep = 360.f / CfgData->ProjectileCount;

    // 360도를 균등 분할해 사방으로 발사
    for (int32 i = 0; i < CfgData->ProjectileCount; ++i)
    {
        const float Angle = AngleStep * i;
        const FRotator Dir(0.f, Angle, 0.f);

        AVSProjectile* Proj = World->SpawnActor<AVSProjectile>(
            CfgData->ProjectileClass, SpawnLoc, Dir);
        if (Proj)
        {
            Proj->Damage = CfgData->ProjectileDamage;
            Proj->Speed = CfgData->ProjectileSpeed;
            Proj->bHitsPlayer = true;   // 플레이어를 타격
        }
    }
}
