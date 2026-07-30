#include "Enemy/VSBossCharger.h"
#include "Data/VSBossData.h"
#include "Character/VSPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

void AVSBossCharger::MoveTowardPlayer(float DeltaTime)
{
    // 돌진 전용 데이터로 캐스팅 (Charger에는 UVSBossChargerData가 지정되어야 함)
    UVSBossChargerData* CfgData = Cast<UVSBossChargerData>(GetData());
    if (!CfgData || !MeshComp) return;

    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player) return;

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
        if (DistToPlayer > CfgData->ContactRange)
            MoveDir = DirToPlayer;

        // 사거리 안 + 쿨다운 끝 -> 조준 시작
        if (DistToPlayer <= CfgData->TriggerRange && CooldownTimer <= 0.f)
            EnterState(EChargerState::Aim);
        break;

    case EChargerState::Aim:
        // 멈춰서 조준. 돌진 방향은 조준이 "끝나는 순간" 고정.
        MoveDir = FVector::ZeroVector;
        if (StateTimer >= CfgData->AimTime)
        {
            ChargeDir = DirToPlayer;   // 이 시점의 플레이어 방향으로 고정
            EnterState(EChargerState::Charge);
        }
        break;

    case EChargerState::Charge:
        // 고정된 방향으로 빠르게 직진 (추적하지 않음 -> 피할 수 있음)
        MoveDir = ChargeDir;
        FaceDir = ChargeDir;
        if (StateTimer >= CfgData->Duration)
            EnterState(EChargerState::Recover);
        break;

    case EChargerState::Recover:
        // 돌진 후 잠시 멈춤
        MoveDir = FVector::ZeroVector;
        if (StateTimer >= CfgData->RecoverTime)
        {
            CooldownTimer = CfgData->Cooldown;
            EnterState(EChargerState::Chase);
        }
        break;
    }

    // --- 이동 적용 ---
    if (!MoveDir.IsNearlyZero())
    {
        const float Speed = (State == EChargerState::Charge) ? CfgData->Speed : CfgData->MoveSpeed;
        SetActorLocation(MyLoc + MoveDir * Speed * DeltaTime);
    }

    // --- 회전 (바라볼 방향으로) ---
    if (!FaceDir.IsNearlyZero())
    {
        FRotator TargetRot = FaceDir.Rotation();
        TargetRot.Yaw -= 90.f;   // Manny 메시 정면 축 보정
        const FRotator CurrentRot = MeshComp->GetComponentRotation();
        const float RotInterp = (State == EChargerState::Charge) ? 720.f : RotSpeed;
        MeshComp->SetWorldRotation(FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, RotInterp));
    }

    // --- 접촉 데미지 (돌진 중엔 배율 적용) ---
    if (DistToPlayer < CfgData->ContactRange)
    {
        if (AVSPlayerCharacter* PC = Cast<AVSPlayerCharacter>(Player))
        {
            const float DmgMult = (State == EChargerState::Charge) ? CfgData->DamageMultiplier : 1.f;
            PC->TakeDamageFromEnemy(CfgData->ContactDamage * DmgMult * DeltaTime);
        }
    }
}

void AVSBossCharger::EnterState(EChargerState NewState)
{
    State = NewState;
    StateTimer = 0.f;
}
