#include "Weapon/VSShieldArea.h"

AVSShieldArea::AVSShieldArea()
{
    PrimaryActorTick.bCanEverTick = false;

    AreaMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AreaMesh"));
    RootComponent = AreaMesh;
    AreaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AVSShieldArea::SetRadius(float Radius)
{
    if (!AreaMesh || !AreaMesh->GetStaticMesh()) return;

    // 메쉬의 실제 반경 구하기
    const FBox Bounds = AreaMesh->GetStaticMesh()->GetBoundingBox();
    const float MeshRadius = Bounds.GetExtent().X;

    if (MeshRadius <= 0.f) return;   // 0 나누기 방지

    // 스케일 계산
    const float Scale = Radius / MeshRadius;
    AreaMesh->SetWorldScale3D(FVector(Scale, Scale, 0.05f));
}