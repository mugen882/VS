#include "Enemy/VSBossRanger.h"
#include "Data/VSBossData.h"
#include "Weapon/VSProjectile.h"
#include "Character/VSPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

void AVSBossRanger::MoveTowardPlayer(float DeltaTime)
{
    UVSBossRangerData* CfgData = Cast<UVSBossRangerData>(GetData());
    if (!CfgData || !MeshComp) return;

    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player) return;

    const FVector MyLoc = GetActorLocation();
    const FVector PlayerLoc = Player->GetActorLocation();
    const FVector ToPlayer = PlayerLoc - MyLoc;
    const FVector DirToPlayer = ToPlayer.GetSafeNormal2D();
    const float DistToPlayer = ToPlayer.Size2D();

    // --- 카이팅 이동 ---
    FVector MoveDir = FVector::ZeroVector;
    if (DistToPlayer < CfgData->FleeRange)
    {
        MoveDir = -DirToPlayer;   // 너무 가까움 -> 물러남
    }
    else if (DistToPlayer > CfgData->KeepDistance)
    {
        MoveDir = DirToPlayer;    // 너무 멈 -> 다가감
    }
    // 그 사이면 멈춤 (적정 거리 유지)

    if (!MoveDir.IsNearlyZero())
        SetActorLocation(MyLoc + MoveDir * CfgData->MoveSpeed * DeltaTime);

    // --- 회전: 항상 플레이어를 바라봄 ---
    if (!DirToPlayer.IsNearlyZero())
    {
        FRotator TargetRot = DirToPlayer.Rotation();
        TargetRot.Yaw -= 90.f;   // Manny 메시 정면 축 보정
        const FRotator CurrentRot = MeshComp->GetComponentRotation();
        MeshComp->SetWorldRotation(FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, RotSpeed));
    }

    // --- 접촉 데미지 (가까이 붙으면) ---
    if (DistToPlayer < CfgData->ContactRange)
    {
        if (AVSPlayerCharacter* PC = Cast<AVSPlayerCharacter>(Player))
            PC->TakeDamageFromEnemy(CfgData->ContactDamage * DeltaTime);
    }
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
