#include "VSOrbitProjectile.h"
#include "Weapon/VSOrbitProjectile.h"

AVSOrbitProjectile::AVSOrbitProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AVSOrbitProjectile::BeginPlay()
{
	Super::BeginPlay();

	Mesh->SetRelativeScale3D(FVector(Scale));
}
