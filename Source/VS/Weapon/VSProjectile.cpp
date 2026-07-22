#include "VSProjectile.h"
#include "Manager/VSEnemyManager.h"
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
}

void AVSProjectile::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 앞으로 이동
    const FVector NewLoc = GetActorLocation() + GetActorForwardVector() * Speed * DeltaTime;
    SetActorLocation(NewLoc);

    // 근처 적 검사
    AVSEnemyManager* EnemyManager = Cast<AVSEnemyManager>(UGameplayStatics::GetActorOfClass(this, AVSEnemyManager::StaticClass()));
    if (EnemyManager)
    {
        FVector HitLoc;
        const int32 Idx = EnemyManager->FindNearestEnemy(GetActorLocation(), HitRadius, HitLoc);
        if (Idx != INDEX_NONE)
        {
            EnemyManager->ApplyDamageToEnemy(Idx, Damage);
            Destroy();   // 명중하면 소멸 (관통 없음)
            return;
        }
    }
}