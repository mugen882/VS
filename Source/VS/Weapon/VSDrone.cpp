#include "VSDrone.h"
#include "Components/SkeletalMeshComponent.h"

AVSDrone::AVSDrone()
{
    PrimaryActorTick.bCanEverTick = false;   // 위치/발사는 UVSDroneBehavior가

    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    RootComponent = SkeletalMesh;
    SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AVSDrone::BeginPlay()
{
    Super::BeginPlay();

    if (SkeletalMesh)
    {
       SkeletalMesh->SetReceivesDecals(false);
    }
}