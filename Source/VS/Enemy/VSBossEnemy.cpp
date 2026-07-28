#include "Enemy/VSBossEnemy.h"
#include "Data/VSBossData.h"
#include "Character/VSPlayerCharacter.h"
#include "Manager/VSGemManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AVSBossEnemy::AVSBossEnemy()
{
    PrimaryActorTick.bCanEverTick = true;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(Root);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    MeshComp->SetWorldScale3D(FVector(1.5f));
    MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
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

        if (Data->AnimClass)
        {
            MeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
            MeshComp->SetAnimInstanceClass(Data->AnimClass);
        }   
    }

    OnBossHealthChanged.Broadcast(GetHealthPercent());
}

void AVSBossEnemy::ReceiveDamage(float Damage)
{
    if (Health <= 0.f) return;

    Health -= Damage;
    OnBossHealthChanged.Broadcast(GetHealthPercent());

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
    UpdateAttack(DeltaTime);   // 파생 공격 패턴
}

void AVSBossEnemy::MoveTowardPlayer(float DeltaTime)
{
    if (!Data) return;

    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player) return;

    const FVector MyLoc = GetActorLocation();
    const FVector PlayerLoc = Player->GetActorLocation();
    const FVector Dir = (PlayerLoc - MyLoc).GetSafeNormal2D();

    // 이동
    SetActorLocation(MyLoc + Dir * Data->MoveSpeed * DeltaTime);

    // 이동 방향을 바라보게 회전
    if (!Dir.IsNearlyZero())
    {
        const FRotator TargetRot = Dir.Rotation();
        SetActorRotation(TargetRot);
    }

    // 접촉 데미지 (초당)
    if (FVector::DistSquared2D(MyLoc, PlayerLoc) < Data->ContactRange * Data->ContactRange)
    {
        if (AVSPlayerCharacter* PC = Cast<AVSPlayerCharacter>(Player))
            PC->TakeDamageFromEnemy(Data->ContactDamage * DeltaTime);
    }
}

void AVSBossEnemy::OnDeath()
{
    // 큰 젬 드롭
    if (Data)
    {
        if (AVSGemManager* GemMgr = Cast<AVSGemManager>(
            UGameplayStatics::GetActorOfClass(this, AVSGemManager::StaticClass())))
        {
            GemMgr->SpawnGem(GetActorLocation(), Data->XPValue);
        }
    }

    OnBossDied.Broadcast();
    Destroy();
}
