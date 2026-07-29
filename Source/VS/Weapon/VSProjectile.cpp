#include "VSProjectile.h"
#include "Manager/VSEnemyManager.h"
#include "Enemy/VSBossEnemy.h"
#include "Character/VSPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

AVSProjectile::AVSProjectile()
{
    PrimaryActorTick.bCanEverTick = true;

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RootComponent = MeshComp;
}

void AVSProjectile::BeginPlay()
{
    Super::BeginPlay();

    if (MeshComp)
    {
        MeshComp->SetRelativeScale3D(FVector(MeshScale));
    }

    SetLifeSpan(LifeSpanSeconds);
}

void AVSProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 앞으로 이동
    const FVector NewLoc = GetActorLocation() + GetActorForwardVector() * Speed * DeltaTime;
    SetActorLocation(NewLoc);

    if (bHitsPlayer)
    {
        // 보스/적 투사체: 플레이어 타격
        APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
        if (Player)
        {
            const float DistSq = FVector::DistSquared2D(GetActorLocation(), Player->GetActorLocation());
            if (DistSq < HitRadius * HitRadius)
            {
                if (AVSPlayerCharacter* PC = Cast<AVSPlayerCharacter>(Player))
                    PC->TakeDamageFromEnemy(Damage);
                Destroy();
                return;
            }
        }
    }
    else
    {
        // 무기 투사체: 가장 가까운 적 타격
        AVSEnemyManager* EnemyManager = Cast<AVSEnemyManager>(UGameplayStatics::GetActorOfClass(this, AVSEnemyManager::StaticClass()));
        if (EnemyManager)
        {
            FVector HitLoc;
            const int32 Idx = EnemyManager->FindNearestEnemy(GetActorLocation(), HitRadius, HitLoc);
            if (Idx == BOSS_TARGET_INDEX)
            {
                // 보스가 최근접 타겟
                float BossDistSq;
                if (AVSBossEnemy* Boss = EnemyManager->FindNearestBoss(GetActorLocation(), HitRadius, BossDistSq))
                {
                    Boss->ReceiveDamage(Damage);
                    Destroy();
                    return;
                }
            }
            else if (Idx != INDEX_NONE)
            {
                EnemyManager->ApplyDamageToEnemy(Idx, Damage);
                Destroy();
                return;
            }
        }
    }
}