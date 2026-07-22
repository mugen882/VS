#include "VSGemManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Character/VSCharacter.h"

AVSGemManager::AVSGemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    GemISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GemISM"));
    RootComponent = GemISM;

    GemISM->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AVSGemManager::BeginPlay()
{
    Super::BeginPlay();
}

void AVSGemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

	UpdateGems(DeltaTime);
}

void AVSGemManager::SpawnGem(const FVector& Location, int32 XPValue)
{
    FVector GemLoc = Location;

    // 메시 바운드에서 절반 높이 구하기
    if (UStaticMesh* Mesh = GemISM->GetStaticMesh())
    {
        const FBox Bounds = Mesh->GetBoundingBox();  // 로컬 바운딩 박스
        const float HalfHeightZ = Bounds.GetExtent().Z;  // Z 반높이

        // 인스턴스 스케일도 반영
        GemLoc.Z = Location.Z + HalfHeightZ * MeshScale;
    }

    // 데이터 추가
    FVSGemData Data;
    Data.Location = GemLoc;
    Data.XPValue = XPValue;
    Data.bBeingCollected = false;
    Gems.Add(Data);

    // ISM 인스턴스 추가 (데이터와 같은 인덱스)
    const FTransform Xform(FRotator::ZeroRotator, Data.Location, FVector(MeshScale));
    GemISM->AddInstance(Xform, /*bWorldSpace=*/true);
}

void AVSGemManager::RemoveGem(int32 Index)
{
    if (!Gems.IsValidIndex(Index)) return;

    const int32 LastIndex = Gems.Num() - 1;

    // 데이터 배열 swap-remove
    Gems.RemoveAtSwap(Index);

    // ISM도 동일하게: 마지막 인스턴스를 죽은 자리로 옮기고 마지막 제거
    if (Index != LastIndex)
    {
        FTransform LastXform;
        GemISM->GetInstanceTransform(LastIndex, LastXform, /*bWorldSpace=*/true);
        GemISM->UpdateInstanceTransform(Index, LastXform, true, true, true);
    }
    GemISM->RemoveInstance(LastIndex);
}

void AVSGemManager::UpdateGems(float DeltaTime)
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player) return;
    const FVector PlayerLoc = Player->GetActorLocation();

    TArray<int32> Collected;
    TArray<FTransform> NewXforms;
    NewXforms.Reserve(Gems.Num());

    float MagnetRangeSq = MagnetRange * MagnetRange;
    for (int32 i = 0; i < Gems.Num(); ++i)
    {
        FVector& Loc = Gems[i].Location;
        const float DistSq = FVector::DistSquared2D(Loc, PlayerLoc);

        // 자석 범위 안이면 흡수 시작
        if (DistSq < MagnetRangeSq)
        {
            Gems[i].bBeingCollected = true;
        }

        // 흡수 중이면 플레이어 쪽으로 이동
        if (Gems[i].bBeingCollected)
        {
            const FVector Dir = (PlayerLoc - Loc).GetSafeNormal();
            Loc += Dir * GemSpeed * DeltaTime;

            // 획득 완료
            if (DistSq < CollectRange * CollectRange)
            {
                APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
                if (AVSCharacter* PC = Cast<AVSCharacter>(PlayerPawn))
                {
                    PC->AddXP(Gems[i].XPValue);
                }
                Collected.Add(i);
            }
        }

        NewXforms.Add(FTransform(FRotator::ZeroRotator, Loc, FVector(MeshScale)));
    }

    // 트랜스폼 일괄 갱신
    if (Gems.Num() > 0)
        GemISM->BatchUpdateInstancesTransforms(0, NewXforms, true, true, true);

    // 획득한 젬 제거 (큰 인덱스부터)
    Collected.Sort([](int32 A, int32 B) { return A > B; });
    for (int32 Idx : Collected)
        RemoveGem(Idx);
}