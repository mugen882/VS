#include "Enemy/VSBossCharger.h"
#include "Data/VSBossData.h"
#include "Weapon/VSProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "Common/VSDefine.h"

void AVSBossCharger::BeginPlay()
{
    Super::BeginPlay();

    // 스폰 후 일정 시간뒤부터 Projectile발사
    FireTimer = BOSS_ATTACK_START_TIME;
}

void AVSBossCharger::UpdateAttack(float DeltaTime)
{
    UVSBossData* BossData = GetData();
    if (!BossData) return;

    FireTimer -= DeltaTime;
    if (FireTimer <= 0.f)
    {
        FireAtPlayer();
        FireTimer = BossData->FireInterval;
    }
}

void AVSBossCharger::FireAtPlayer()
{
    UVSBossData* BossData = GetData();
    if (!BossData || !BossData->ProjectileClass) return;

    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player) return;

    const FVector MyLoc = GetActorLocation();
    const FVector Dir = (Player->GetActorLocation() - MyLoc).GetSafeNormal2D();

    AVSProjectile* Proj = GetWorld()->SpawnActor<AVSProjectile>(
        BossData->ProjectileClass, MyLoc, Dir.Rotation());
    if (Proj)
    {
        Proj->Damage = BossData->ProjectileDamage;
        Proj->bHitsPlayer = true;   // 플레이어를 타격
    }
}
