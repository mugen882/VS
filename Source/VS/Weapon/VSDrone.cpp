#include "VSDrone.h"
#include "Components/StaticMeshComponent.h"

AVSDrone::AVSDrone()
{
    PrimaryActorTick.bCanEverTick = false;   // 위치/발사는 WeaponComponent가

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}