#include "Debug/VSBenchmarkDummy.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

AVSBenchmarkDummy::AVSBenchmarkDummy()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(Root);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComp->SetReceivesDecals(false);
    MeshComp->SetRelativeRotation(FRotator(0.f, MeshYawOffset, 0.f));

    // ISM 경로와 조건을 맞춘다: 화면 밖이어도 포즈를 계속 갱신
    MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}

void AVSBenchmarkDummy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!CachedPlayer.IsValid())
    {
        CachedPlayer = UGameplayStatics::GetPlayerPawn(this, 0);
    }
    const APawn* Player = CachedPlayer.Get();
    if (!Player) return;

    const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
    const FVector Dir = ToPlayer.GetSafeNormal2D();
    if (Dir.IsNearlyZero()) return;

    SetActorLocation(GetActorLocation() + Dir * MoveSpeed * DeltaTime);
    SetActorRotation(Dir.Rotation());
}
