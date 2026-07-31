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

AVSBossEnemy::AVSBossEnemy()
{
    PrimaryActorTick.bCanEverTick = true;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(Root);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    MeshComp->SetWorldScale3D(FVector(BOSS_ENEMY_SCALE));
    MeshComp->SetReceivesDecals(false);   // 자신의 텔레그래프 데칼이 몸에 묻지 않게

    // 머리 위 체력바 — 화면을 향하는 위젯. 위젯 클래스는 BP에서 지정
    HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
    HealthBarWidget->SetupAttachment(Root);
    HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);   // 항상 카메라 향함
    // 생성자 시점엔 메시가 아직 없어 바운드가 무의미하다. 실제 높이는 InitBoss에서 계산.
    HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, BOSS_HEADBAR_FALLBACK_HEIGHT));
    HealthBarWidget->SetDrawAtDesiredSize(true);
}

void AVSBossEnemy::InitBoss(UVSBossData* InData)
{
    Data = InData;
    if (Data && MeshComp)
    {
        Health = Data->MaxHealth;

        if (Data->Mesh)
        {
            MeshComp->SetSkeletalMesh(Data->Mesh);
        }

        // 메시 축 보정은 메시와 한 세트. 회전은 액터가 담당하므로 보정 지점은 여기뿐이다
        MeshComp->SetRelativeRotation(FRotator(0.f, Data->MeshYawOffset, 0.f));

        if (Data->AnimClass)
        {
            MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
            MeshComp->SetAnimInstanceClass(Data->AnimClass);
        }   

        CacheData();   // 메시가 정해진 뒤에야 바운드가 의미를 가진다
    }

    OnBossHealthChanged.Broadcast(GetHealthPercent());
}

void AVSBossEnemy::ReceiveDamage(float Damage)
{
    if (Health <= 0.f) return;

    Health -= Damage;
    OnBossHealthChanged.Broadcast(GetHealthPercent());
    OnBossDamaged.Broadcast(this);   // 화면 상단 바가 "마지막 타격 보스"로 이 보스를 추적

    if (Health <= 0.f)
    {
        Health = 0.f;
        OnDeath();
    }
}

float AVSBossEnemy::GetHealthPercent() const
{
    const float Max = (Data && Data->MaxHealth > 0.f) ? Data->MaxHealth : 1.f;
    return FMath::Clamp(Health / Max, 0.f, 1.f);
}

void AVSBossEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (Health <= 0.f) return;

    MoveTowardPlayer(DeltaTime);
    UpdateAttack(DeltaTime);

    UpdateHealthBarPosition(DeltaTime);
}

void AVSBossEnemy::UpdateHealthBarPosition(float DeltaTime)
{
    if (!HealthBarWidget || !MeshComp) return;

    APlayerCameraManager* Cam = UGameplayStatics::GetPlayerCameraManager(this, 0);
    if (!Cam) return;

    // 카메라의 "위쪽" 방향으로 밀면 화면상 위로 간다.
    const FVector CamUp = Cam->GetCameraRotation().RotateVector(FVector::UpVector);

    // 보스 머리 근처 + 화면상 위로 오프셋
    const FVector TargetLoc = GetActorLocation()
        + FVector(0.f, 0.f, HeadHeight)   // 머리 높이 (InitBoss에서 바운드로 계산)
        + CamUp * ScreenUpOffset;         // 카메라 기준 화면상 위로

    // 부드럽게 이동 (급격한 튐/흔들림 방지)
    const FVector Current = HealthBarWidget->GetComponentLocation();
    HealthBarWidget->SetWorldLocation(FMath::VInterpTo(Current, TargetLoc, DeltaTime, 20.f));
}

void AVSBossEnemy::CacheData()
{
    if (!MeshComp) return;

    // 메시를 막 교체한 직후라 바운드가 아직 갱신 전일 수 있다
    MeshComp->UpdateBounds();

    // 월드 바운드 상단 - 액터 기준점 = 발끝에서 머리끝까지 (컴포넌트 스케일 포함)
    const FBoxSphereBounds& MeshBounds = MeshComp->Bounds;
    const float Top = MeshBounds.Origin.Z + MeshBounds.BoxExtent.Z;
    const float Height = Top - GetActorLocation().Z;

    HeadHeight = (Height > 0.f) ? Height : BOSS_HEADBAR_FALLBACK_HEIGHT;

    if (HealthBarWidget)
        HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, HeadHeight));

    const float Radius = FMath::Max(MeshBounds.BoxExtent.X, MeshBounds.BoxExtent.Y);
    MeshRadius = (Radius > 0.f) ? Radius : BOSS_MESH_RADIUS;
}

void AVSBossEnemy::MoveTowardPlayer(float DeltaTime)
{
    const FVSBossPlayerInfo Info = QueryPlayer();
    if (!Info.IsValid()) return;

    // 접촉 사거리 밖일 때만 접근 (오버슈트로 인한 요동 방지)
    if (Info.Dist > Data->ContactRange)
        MoveInDirection(Info.Dir, Data->MoveSpeed, DeltaTime);

    FaceDirection(Info.Dir, GetRotateSpeedDeg(), DeltaTime);
    ApplyContactDamage(DeltaTime);
}

FVSBossPlayerInfo AVSBossEnemy::QueryPlayer() const
{
    FVSBossPlayerInfo Info;
    if (!Data) return Info;

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
        HealthBarWidget->InitWidget();   // 위젯이 아직 생성 전일 수 있어 강제 초기화
        if (UVSBossHeadBarWidget* HeadBar = Cast<UVSBossHeadBarWidget>(HealthBarWidget->GetWidget()))
        {
            UVSBossHeadBarViewModel* VM = NewObject<UVSBossHeadBarViewModel>(this);
            VM->BindBoss(this);
            HeadBar->SetViewModel(VM);   // → OnViewModelSet
        }
    }
}

void AVSBossEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (EnemyManager.IsValid())
        EnemyManager->UnregisterBoss(this);

    Super::EndPlay(EndPlayReason);
}

void AVSBossEnemy::OnDeath()
{
    if (Data)
    {
        if (AVSGemManager* GemMgr = Cast<AVSGemManager>(
            UGameplayStatics::GetActorOfClass(this, AVSGemManager::StaticClass())))
        {
            GemMgr->SpawnGem(GetActorLocation(), Data->XPValue);
        }
    }

    OnBossDied.Broadcast(this);
    Destroy();
}
