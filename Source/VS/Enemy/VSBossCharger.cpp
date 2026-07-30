#include "Enemy/VSBossCharger.h"
#include "Data/VSBossData.h"
#include "Character/VSPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

void AVSBossCharger::MoveTowardPlayer(float DeltaTime)
{
    UVSBossData* BossData = GetData();
    if (!BossData || !MeshComp) return;

    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player) return;

    const FVSChargeConfig& Cfg = BossData->Charge;

    const FVector MyLoc = GetActorLocation();
    const FVector PlayerLoc = Player->GetActorLocation();
    const FVector ToPlayer = PlayerLoc - MyLoc;
    const FVector DirToPlayer = ToPlayer.GetSafeNormal2D();
    const float DistToPlayer = ToPlayer.Size2D();

    StateTimer += DeltaTime;
    if (CooldownTimer > 0.f)
        CooldownTimer -= DeltaTime;

    // --- 상태별 이동 ---
    FVector MoveDir = FVector::ZeroVector;   // 이번 프레임 이동 방향
    FVector FaceDir = DirToPlayer;           // 바라볼 방향 (기본: 플레이어)

    switch (State)
    {
    case EChargerState::Chase:
        // 플레이어를 향해 천천히 추격 (접촉 사거리 밖일 때만 — 요동 방지)
        if (DistToPlayer > BossData->ContactRange)
            MoveDir = DirToPlayer;

        // 사거리 안 + 쿨다운 끝 -> 조준 시작
        if (DistToPlayer <= Cfg.TriggerRange && CooldownTimer <= 0.f)
            EnterState(EChargerState::Aim);
        break;

    case EChargerState::Aim:
        // 멈춰서 조준. 돌진 방향은 조준이 "끝나는 순간" 고정.
        MoveDir = FVector::ZeroVector;
        if (StateTimer >= Cfg.AimTime)
        {
            ChargeDir = DirToPlayer;   // 이 시점의 플레이어 방향으로 고정
            EnterState(EChargerState::Charge);
        }
        break;

    case EChargerState::Charge:
        // 고정된 방향으로 빠르게 직진 (추적하지 않음 -> 피할 수 있음)
        MoveDir = ChargeDir;
        FaceDir = ChargeDir;
        if (StateTimer >= Cfg.Duration)
            EnterState(EChargerState::Recover);
        break;

    case EChargerState::Recover:
        // 돌진 후 잠시 멈춤
        MoveDir = FVector::ZeroVector;
        if (StateTimer >= Cfg.RecoverTime)
        {
            CooldownTimer = Cfg.Cooldown;
            EnterState(EChargerState::Chase);
        }
        break;
    }

    // --- 이동 적용 ---
    if (!MoveDir.IsNearlyZero())
    {
        const float Speed = (State == EChargerState::Charge) ? Cfg.Speed : BossData->MoveSpeed;
        SetActorLocation(MyLoc + MoveDir * Speed * DeltaTime);
    }

    // --- 회전 (바라볼 방향으로) ---
    if (!FaceDir.IsNearlyZero())
    {
        FRotator TargetRot = FaceDir.Rotation();
        TargetRot.Yaw -= 90.f;   // Manny 메시 정면 축 보정
        const FRotator CurrentRot = MeshComp->GetComponentRotation();
        MeshComp->SetWorldRotation(FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, RotSpeed));
    }

    // --- 접촉 데미지 (돌진 중엔 배율 적용) ---
    if (DistToPlayer < BossData->ContactRange)
    {
        if (AVSPlayerCharacter* PC = Cast<AVSPlayerCharacter>(Player))
        {
            const float DmgMult = (State == EChargerState::Charge) ? Cfg.DamageMultiplier : 1.f;
            PC->TakeDamageFromEnemy(BossData->ContactDamage * DmgMult * DeltaTime);
        }
    }
}

void AVSBossCharger::EnterState(EChargerState NewState)
{
    State = NewState;
    StateTimer = 0.f;
}
