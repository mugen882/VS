#include "Enemy/VSBossEnemy.h"
#include "Data/VSBossData.h"
#include "Character/VSPlayerCharacter.h"
#include "Manager/VSGemManager.h"
#include "Manager/VSEnemyManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/VSBossHeadBarWidget.h"
#include "ViewModel/VSBossHeadBarViewModel.h"
#include "Kismet/GameplayStatics.h"
#include "Component/VSWeaponComponent.h"
#include "Common/VSLog.h"

namespace
{
    const FName TintParamName(TEXT("TeamColor"));

    // 체력바 위치 추종 속도.
    constexpr float HealthBarInterpSpeed = 20.f;
}

AVSBossEnemy::AVSBossEnemy()
{
    PrimaryActorTick.bCanEverTick = true;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(Root);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    MeshComp->SetRelativeScale3D(FVector(BOSS_ENEMY_SCALE));
    MeshComp->SetReceivesDecals(false);   // 자신의 텔레그래프 데칼이 몸에 묻지 않게

    // 머리 위 체력바 — 화면을 향하는 위젯.
    HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
    HealthBarWidget->SetupAttachment(Root);
    HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
    HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, BOSS_HEADBAR_FALLBACK_HEIGHT));
    HealthBarWidget->SetDrawAtDesiredSize(true);
}

void AVSBossEnemy::InitBoss(UVSBossData* InData)
{
    if (!InData)
    {
        UE_LOG(VSLog, Warning, TEXT("InitBoss: BossData가 비어 있습니다. 웨이브의 BossData 지정을 확인하세요."));
        Destroy();
        return;
    }

    Data = InData;
    if (Data && MeshComp)
    {
        Health = Data->MaxHealth;

        if (Data->Mesh)
        {
            MeshComp->SetSkeletalMesh(Data->Mesh);
        }

        ApplyTint(Data->Tint);

        // 메시 축 보정은 메시와 한 세트. 회전은 액터가 담당하므로 보정 지점은 여기뿐이다
        MeshComp->SetRelativeRotation(FRotator(0.f, Data->MeshYawOffset, 0.f));

        if (Data->AnimClass)
        {
            MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
            MeshComp->SetAnimInstanceClass(Data->AnimClass);
        }   

        CacheData();   // 메시가 정해진 뒤에야 바운드가 의미를 가진다
    }

    FVector Target;
    if (ComputeHealthBarTarget(Target))
    {
        HealthBarWidget->SetWorldLocation(Target);
    }

    OnBossHealthChanged.Broadcast(GetHealthPercent());
}

void AVSBossEnemy::ReceiveDamage(float Damage)
{
    if (bDead) return;

    Health -= Damage;
    OnBossHealthChanged.Broadcast(GetHealthPercent());
    OnBossDamaged.Broadcast(this);   // 화면 상단 바가 "마지막 타격 보스"로 이 보스를 추적

    if (Health <= 0.f)
    {
        Health = 0.f;
        HandleDeath(true);
    }
}

void AVSBossEnemy::Kill(bool bInGrantRewards)
{
    if (bDead) return;

    Health = 0.f;
    OnBossHealthChanged.Broadcast(0.f);

    HandleDeath(bInGrantRewards);
}

void AVSBossEnemy::HandleDeath(bool bInGrantRewards)
{
    if (bDead) return;   // 사망 처리는 정확히 1회

    bDead = true;
    bGrantRewards = bInGrantRewards;

    OnDeath();   // 파생이 자기 정리 후 Super::OnDeath 호출
}

float AVSBossEnemy::GetHealthPercent() const
{
    const float Max = (Data && Data->MaxHealth > 0.f) ? Data->MaxHealth : 1.f;
    return FMath::Clamp(Health / Max, 0.f, 1.f);
}

void AVSBossEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDead) return;

    MoveTowardPlayer(DeltaTime);
    ApplyContactDamage(DeltaTime, GetContactDamageMultiplier());
    UpdateAttack(DeltaTime);

    UpdateHealthBarPosition(DeltaTime);
}

bool AVSBossEnemy::ComputeHealthBarTarget(FVector& OutTarget) const
{
    if (!HealthBarWidget || !MeshComp) return false;

    APlayerCameraManager* Cam = UGameplayStatics::GetPlayerCameraManager(this, 0);
    if (!Cam) return false;

    // 카메라의 "위쪽" 방향으로 밀면 화면상 위로 간다.
    const FVector CamUp = Cam->GetCameraRotation().RotateVector(FVector::UpVector);

    OutTarget = GetActorLocation()
        + FVector(0.f, 0.f, HeadHeight)
        + CamUp * ScreenUpOffset;
    return true;
}

void AVSBossEnemy::UpdateHealthBarPosition(float DeltaTime)
{
    if (!HealthBarWidget) return;

    // 보스 머리 근처 + 화면상 위로 오프셋
    FVector TargetLoc;
    if (ComputeHealthBarTarget(TargetLoc))
    {
        // 부드럽게 이동 (급격한 튐/흔들림 방지)
        const FVector Current = HealthBarWidget->GetComponentLocation();
        HealthBarWidget->SetWorldLocation(FMath::VInterpTo(Current, TargetLoc, DeltaTime, HealthBarInterpSpeed));
    }
}

void AVSBossEnemy::CacheData()
{
    if (!MeshComp) return;

    // 메시를 막 교체한 직후라 바운드가 아직 갱신 전일 수 있다
    MeshComp->UpdateBounds();

    const FBoxSphereBounds& MeshBounds = MeshComp->Bounds;
    const float Height = MeshBounds.GetBox().Max.Z - GetActorLocation().Z;

    HeadHeight = (Height > 0.f) ? Height : BOSS_HEADBAR_FALLBACK_HEIGHT;

    const float Radius = FMath::Max(MeshBounds.BoxExtent.X, MeshBounds.BoxExtent.Y);
    MeshRadius = (Radius > 0.f) ? Radius : BOSS_MESH_RADIUS;
}

void AVSBossEnemy::MoveTowardPlayer(float DeltaTime)
{
    const FVSBossPlayerInfo Info = QueryPlayer();
    if (!Info.IsValid()) return;
    if (!Data) return;

    // 접촉 사거리 밖일 때만 접근 (오버슈트로 인한 요동 방지)
    if (Info.Dist > Data->ContactRange)
        MoveInDirection(Info.Dir, Data->MoveSpeed, DeltaTime);

    FaceDirection(Info.Dir, GetRotateSpeedDeg(), DeltaTime);
}

FVSBossPlayerInfo AVSBossEnemy::QueryPlayer() const
{
    FVSBossPlayerInfo Info;

    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player) return Info;

    const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
    Info.Pawn = Player;
    Info.Dir  = ToPlayer.GetSafeNormal2D();
    Info.Dist = ToPlayer.Size2D();
    return Info;
}

void AVSBossEnemy::MoveInDirection(const FVector& Dir, float Speed, float DeltaTime)
{
    if (Dir.IsNearlyZero()) return;
    SetActorLocation(GetActorLocation() + Dir * Speed * DeltaTime);
}

void AVSBossEnemy::ApplyKiting(const FVSBossPlayerInfo& Info, float DeltaTime)
{
    if (!Data || !Info.IsValid()) return;

    FVector MoveDir = FVector::ZeroVector;
    if (Info.Dist < Data->FleeRange)
    {
        MoveDir = -Info.Dir;          // 너무 가까움 -> 물러남
    }
    else if (Info.Dist > Data->KeepDistance)
    {
        MoveDir = Info.Dir;           // 너무 멀다 -> 다가감
    }
    // 그 사이면 정지 (적정 거리 유지)

    MoveInDirection(MoveDir, Data->MoveSpeed, DeltaTime);
}

void AVSBossEnemy::FaceDirection(const FVector& Dir, float DegPerSec, float DeltaTime)
{
    if (Dir.IsNearlyZero()) return;

    // 등속 회전(도/초).
    const FRotator TargetRot = Dir.Rotation();
    SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(), TargetRot, DeltaTime, DegPerSec));
}

void AVSBossEnemy::ApplyContactDamage(float DeltaTime, float DamageMult)
{
    if (!Data) return;

    const FVSBossPlayerInfo Info = QueryPlayer();
    if (!Info.IsValid()) return;

    // 플레이어 몸 반경까지 포함해야 데칼 영역내에 있을때 몸이 닿으면 맞는다.
    const float HitRange = Data->ContactRange + Info.Pawn->GetSimpleCollisionRadius();
    if (Info.Dist >= HitRange) return;

    if (AVSPlayerCharacter* PC = Cast<AVSPlayerCharacter>(Info.Pawn))
        PC->TakeDamageFromEnemy(Data->ContactDamage * DamageMult * DeltaTime);
}

float AVSBossEnemy::GetRotateSpeedDeg() const
{
    return Data ? Data->RotateSpeedDeg : BOSS_ROTATE_SPEED_DEG;
}

void AVSBossEnemy::BeginPlay()
{
    Super::BeginPlay();

    if (AVSPlayerCharacter* Player = Cast<AVSPlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
    {
        if (UVSWeaponComponent* WeaponComp = Player->GetWeaponComponent())
        {
            EnemyManager = WeaponComp->GetEnemyManager();
            if (EnemyManager.IsValid())
                EnemyManager->RegisterBoss(this);
        }
    }

    // 머리 위 체력바: 뷰모델 생성 → 자기 보스와 연결 → 위젯에 주입
    if (HealthBarWidget)
    {
        HealthBarWidget->InitWidget();
        if (UVSBossHeadBarWidget* HeadBar = Cast<UVSBossHeadBarWidget>(HealthBarWidget->GetWidget()))
        {
            HeadBarViewModel = NewObject<UVSBossHeadBarViewModel>(this);
            HeadBarViewModel->BindBoss(this);
            HeadBar->SetViewModel(HeadBarViewModel);
        }
    }
}

void AVSBossEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // EndPlay는 사망 외에 레벨 전환·PIE 종료로도 불린다.
    // 사망 통지는 OnDeath가 책임지고, 여기서는 등록 해제만 한다.
    if (EnemyManager.IsValid())
        EnemyManager->UnregisterBoss(this);

    Super::EndPlay(EndPlayReason);
}

void AVSBossEnemy::OnDeath()
{
    if (ShouldGrantRewards() && Data)
    {
        if (AVSPlayerCharacter* Player = Cast<AVSPlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
        {
            if (AVSGemManager * GemMgr = Player->GetGemManager())
            {
                GemMgr->SpawnGem(GetActorLocation(), Data->XPValue);
            }
        }
    }

    OnBossDied.Broadcast(this);
    Destroy();
}

void AVSBossEnemy::ApplyTint(const FLinearColor& Tint)
{
    if (!MeshComp) return;

    // 슬롯마다 MID를 한 번만 만들어 캐시한다
    if (BodyMIDs.Num() == 0)
    {
        const int32 NumMats = MeshComp->GetNumMaterials();
        BodyMIDs.Reserve(NumMats);

        for (int32 i = 0; i < NumMats; ++i)
        {
            if (UMaterialInstanceDynamic* MID = MeshComp->CreateDynamicMaterialInstance(i))
            {
                BodyMIDs.Add(MID);
            }
        }
    }

    for (UMaterialInstanceDynamic* MID : BodyMIDs)
    {
        MID->SetVectorParameterValue(TintParamName, Tint);
    }
}