#include "Enemy/VSBossCharger.h"
#include "Data/VSBossData.h"
#include "Components/DecalComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

AVSBossCharger::AVSBossCharger()
{
    TelegraphDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("TelegraphDecal"));
    TelegraphDecal->SetupAttachment(RootComponent);

    // 데칼은 컴포넌트의 -X 방향으로 투영된다. Pitch +90이면 +X가 위 -> 투영이 아래(바닥)를 향한다.
    TelegraphDecal->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
    TelegraphDecal->SetVisibility(false);

    // 화면 점유율이 작아지면 엔진이 데칼을 컬링한다. 예고선은 항상 보여야 하므로 끈다
    TelegraphDecal->FadeScreenSize = 0.f;
}

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

        // 텔레그래프가 차오르는 정도 = 조준 진행도. 플레이어에게 "언제 터지는지"를 알려준다
        UpdateTelegraph(CfgData->AimTime > 0.f ? StateTimer / CfgData->AimTime : 1.f);

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
}

void AVSBossCharger::EnterState(EChargerState NewState)
{
    State = NewState;
    StateTimer = 0.f;

    // 텔레그래프는 조준 중에만 보인다
    if (NewState == EChargerState::Aim)
    {
        SetupTelegraph();
        UpdateTelegraph(0.f);
        ShowTelegraph(true);
    }
    else
    {
        ShowTelegraph(false);
    }
}

void AVSBossCharger::OnDeath()
{
    ShowTelegraph(false);   // 조준 중 죽으면 예고선이 남으므로 지운다
    Super::OnDeath();
}

float AVSBossCharger::GetContactDamageMultiplier() const
{
    const UVSBossChargerData* CfgData = Cast<UVSBossChargerData>(GetData());
    if (!CfgData) return 1.f;

    return (State == EChargerState::Charge) ? CfgData->DamageMultiplier : 1.f;
}

void AVSBossCharger::SetupTelegraph()
{
    UVSBossChargerData* CfgData = Cast<UVSBossChargerData>(GetData());
    if (!CfgData || !TelegraphDecal) return;

    // 머티리얼이 지정되지 않았으면 텔레그래프 기능 자체를 끈다
    if (!CfgData->TelegraphMaterial) return;

    if (!TelegraphMID)
    {
        TelegraphMID = UMaterialInstanceDynamic::Create(CfgData->TelegraphMaterial, this);
        TelegraphDecal->SetDecalMaterial(TelegraphMID);
    }

    // 돌진이 실제로 도달하는 거리. 예고선의 끝은 반드시 여기에 맞춘다
    const float ChargeDist = CfgData->Speed * CfgData->Duration;

    // 보스 발밑은 비운다. 끝점은 그대로 두고 시작점만 앞으로 밀어 길이를 줄인다
    const float Start = FMath::Min(GetMeshRadius() * CfgData->TelegraphStartScale, ChargeDist);
    const float DrawLen = ChargeDist - Start;

    // DecalSize는 반크기. Pitch 90 회전 후 각 축에 대응하는 방향이 바뀜
    TelegraphDecal->DecalSize = FVector(
        CfgData->TelegraphDepth,    // X: 투영 깊이
        CfgData->ContactRange,      // Y: 반폭 = 접촉 판정 반경
        DrawLen * 0.5f);            // Z: 반길이 (액터 전방)

    // 그려지는 구간의 중심으로 이동 (Start ~ ChargeDist 구간의 한가운데)
    TelegraphDecal->SetRelativeLocation(FVector(Start + DrawLen * 0.5f, 0.f, 0.f));
}

void AVSBossCharger::ShowTelegraph(bool bShow)
{
    if (TelegraphDecal)
        TelegraphDecal->SetVisibility(bShow && TelegraphMID != nullptr);
}

void AVSBossCharger::UpdateTelegraph(float Ratio)
{
    if (TelegraphMID)
        TelegraphMID->SetScalarParameterValue(TEXT("Fill"), FMath::Clamp(Ratio, 0.f, 1.f));
}
