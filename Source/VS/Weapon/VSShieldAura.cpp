#include "Weapon/VSShieldAura.h"

AVSShieldAura::AVSShieldAura()
{
    PrimaryActorTick.bCanEverTick = false;

    AuraMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AuraMesh"));
    RootComponent = AuraMesh;
    AuraMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AVSShieldAura::SetRadius(float Radius)
{
    if (!AuraMesh || !AuraMesh->GetStaticMesh()) return;

    // 메쉬의 실제 반경 구하기
    const FBox Bounds = AuraMesh->GetStaticMesh()->GetBoundingBox();
    const float MeshRadius = Bounds.GetExtent().X;

    if (MeshRadius <= 0.f) return;   // 0 나누기 방지

    // 스케일 계산
    const float Scale = Radius / MeshRadius;
    AuraMesh->SetWorldScale3D(FVector(Scale, Scale, 0.05f));
}