#include "Enemy/VSBossCharger.h"
#include "Data/VSBossData.h"

void AVSBossCharger::MoveTowardPlayer(float DeltaTime)
{
    UVSBossChargerData* CfgData = Cast<UVSBossChargerData>(GetData());
    if (!CfgData) return;

    const FVSBossPlayerInfo Info = QueryPlayer();
    if (!Info.IsValid()) return;

    StateTimer += DeltaTime;
    if (CooldownTimer > 0.f)
        CooldownTimer -= DeltaTime;

    FVector MoveDir  = FVector::ZeroVector;      // 이번 프레임 이동 방향
    FVector FaceDir  = Info.Dir;                 // 바라볼 방향
    float MoveSpeed  = CfgData->MoveSpeed;
    float TurnSpeed  = GetRotateSpeedDeg();

    switch (State)
    {
    case EChargerState::Chase:
        // 플레이어를 향해 천천히 추격 (접촉 사거리 밖일 때만)
        if (Info.Dist > CfgData->ContactRange)
            MoveDir = Info.Dir;

        // 사거리 안 + 쿨다운 끝 -> 조준 시작
        if (Info.Dist <= CfgData->TriggerRange && CooldownTimer <= 0.f)
            EnterState(EChargerState::Aim);
        break;

    case EChargerState::Aim:
        // 멈춰서 조준. 이때 선회를 느리게 해야 플레이어가 방향을 읽고 피할 수 있다.
        TurnSpeed = CfgData->AimRotateSpeedDeg;
        if (StateTimer >= CfgData->AimTime)
        {
            ChargeDir = GetActorForwardVector();   // 조준이 끝난 시점의 "실제 바라보는 방향"으로 고정
            EnterState(EChargerState::Charge);
        }
        break;

    case EChargerState::Charge:
        // 고정된 방향으로 빠르게 직진 (이동 중 추적X)
        MoveDir   = ChargeDir;
        FaceDir   = ChargeDir;
        MoveSpeed = CfgData->Speed;
        if (StateTimer >= CfgData->Duration)
            EnterState(EChargerState::Recover);
        break;

    case EChargerState::Recover:
        // 돌진 후 잠시 멈춤
        if (StateTimer >= CfgData->RecoverTime)
        {
            CooldownTimer = CfgData->Cooldown;
            EnterState(EChargerState::Chase);
        }
        break;
    }

    MoveInDirection(MoveDir, MoveSpeed, DeltaTime);
    FaceDirection(FaceDir, TurnSpeed, DeltaTime);

    const float DmgMult = (State == EChargerState::Charge) ? CfgData->DamageMultiplier : 1.f;
    ApplyContactDamage(Info, DeltaTime, DmgMult);
}

void AVSBossCharger::EnterState(EChargerState NewState)
{
    State = NewState;
    StateTimer = 0.f;
}
