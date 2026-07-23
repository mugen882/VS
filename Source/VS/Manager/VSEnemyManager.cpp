#include "VSEnemyManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "VSGemManager.h"
#include "Character/VSCharacter.h"
#include "Data/VSEnemyTypeData.h"
#include "Subsystem/VSDifficultySubsystem.h"

AVSEnemyManager::AVSEnemyManager()
{
    PrimaryActorTick.bCanEverTick = true;

    ISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM"));
    RootComponent = ISM;
    ISM->NumCustomDataFloats = 7; // 인스턴스당 커스텀 데이터
}

void AVSEnemyManager::BeginPlay()
{
    Super::BeginPlay();

    if (ISM)
        ISM->ClearInstances();
    Enemies.Empty();

    GemManager = Cast<AVSGemManager>(UGameplayStatics::GetActorOfClass(this, AVSGemManager::StaticClass()));
}

void AVSEnemyManager::SpawnEnemy(const UVSEnemyTypeData* Type, float HealthMult)
{
    if (!Type || !ISM) return;
    if (Enemies.Num() >= MaxEnemies) return;

    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    const FVector Center = Player ? Player->GetActorLocation() : GetActorLocation();

    // 링 영역에 스폰: 랜덤 각도 + 랜덤 거리(Min~Max)
    const float Angle = FMath::FRandRange(0.f, 2.f * PI);
    const float Dist = FMath::FRandRange(MinSpawnRadius, MaxSpawnRadius);

    const FVector Loc(
        Center.X + FMath::Cos(Angle) * Dist,
        Center.Y + FMath::Sin(Angle) * Dist,
        0.f);

    // 타입 크기 반영
    const FTransform Xform(FRotator::ZeroRotator, Loc, FVector(Type->Scale));
    const int32 Index = ISM->AddInstance(Xform, /*bWorldSpace=*/true);

    // 애니메이션용 커스텀 데이터
    ISM->SetCustomDataValue(Index, (int32)EEnemyCustomData::Anim_StartTime, FMath::FRand());
    ISM->SetCustomDataValue(Index, (int32)EEnemyCustomData::Anim_Speed, 1.0f);          // 달리기 애니 속도 배율
    ISM->SetCustomDataValue(Index, (int32)EEnemyCustomData::Anim_StartFrame, 320.0f);   // 달리기 애니 시작 프레임
    ISM->SetCustomDataValue(Index, (int32)EEnemyCustomData::Anim_EndFrame, 338.0f);     // 달리기 애니 끝 프레임
    // 틴트용 커스텀데이터
    ISM->SetCustomDataValue(Index, (int32)EEnemyCustomData::Tint_R, Type->Tint.R);
    ISM->SetCustomDataValue(Index, (int32)EEnemyCustomData::Tint_G, Type->Tint.G);
    ISM->SetCustomDataValue(Index, (int32)EEnemyCustomData::Tint_B, Type->Tint.B, /*bMark*/ true);   // 마지막만 dirty

    // 개별 스탯을 타입에서 복사 (매 틱 데이터에셋 조회 없이 순회하려고)
    FEnemyData Enemy;
    Enemy.Location = Loc;
    Enemy.Health = Type->MaxHealth * HealthMult;
    Enemy.MoveSpeed = Type->MoveSpeed;
    Enemy.ContactDamage = Type->ContactDamage;
    Enemy.XPValue = Type->XPValue;
    Enemy.Scale = Type->Scale;

    Enemies.Add(Enemy);
}

void AVSEnemyManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateEnemies(DeltaTime);
}

void AVSEnemyManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Warning, TEXT("EnemyManager EndPlay - clearing %d instances"), ISM ? ISM->GetInstanceCount() : -1);
    if (ISM)
        ISM->ClearInstances();

    Enemies.Empty();

    Super::EndPlay(EndPlayReason);
}

int32 AVSEnemyManager::FindNearestEnemy(const FVector& From, float MaxRange, FVector& OutLocation) const
{
    int32 BestIndex = INDEX_NONE;
    float BestDistSq = MaxRange * MaxRange;   // 범위 밖은 무시

    for (int32 i = 0; i < Enemies.Num(); ++i)
    {
        if (Enemies[i].Health <= 0.f) continue;   // 죽은 적 스킵

        const float DistSq = FVector::DistSquared2D(From, Enemies[i].Location);
        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            BestIndex = i;
            OutLocation = Enemies[i].Location;
        }
    }
    return BestIndex;
}

void AVSEnemyManager::ApplyDamageToEnemy(int32 Index, float Damage)
{
    if (!Enemies.IsValidIndex(Index)) return;
    if (Enemies[Index].Health <= 0.f) return;
    if (!GemManager) return;

    Enemies[Index].Health -= Damage;
    if (Enemies[Index].Health <= 0.f)
    {
        KillEnemy(Index);
    }
}

void AVSEnemyManager::KillEnemy(int32 Index)
{
    const int32 LastIndex = Enemies.Num() - 1;
    if (!Enemies.IsValidIndex(Index)) return;

    const FVector DeathLoc = Enemies[Index].Location;
    if (GemManager)
        GemManager->SpawnGem(DeathLoc, Enemies[Index].XPValue);   // 타입별 젬 가치

    // 처치 수 통계 누적
    if (UWorld* World = GetWorld())
    {
        if (UVSDifficultySubsystem* Diff = World->GetSubsystem<UVSDifficultySubsystem>())
            Diff->AddKill();
    }

    // 1. 배열을 swap-remove (마지막을 죽은 자리로, 마지막 제거)
    Enemies[Index] = Enemies[LastIndex];
    Enemies.RemoveAt(LastIndex);

    // 2. ISM도 똑같이: 마지막 인스턴스의 트랜스폼을 죽은 자리로 복사
    if (Index != LastIndex)
    {
        FTransform LastXform;
        ISM->GetInstanceTransform(LastIndex, LastXform, /*bWorldSpace=*/true);
        ISM->UpdateInstanceTransform(Index, LastXform, true, true, true);

        // 커스텀 데이터(TimeOffset 등)도 옮겨야 애니가 이어짐
        // 마지막 인스턴스의 커스텀 데이터를 죽은 자리로 복사
        for (int32 s = 0; s < ISM->NumCustomDataFloats; ++s)
        {
            const float Val = ISM->PerInstanceSMCustomData[LastIndex * ISM->NumCustomDataFloats + s];
            ISM->SetCustomDataValue(Index, s, Val, /*bMark*/ (s == ISM->NumCustomDataFloats - 1));
        }
    }

    // 3. ISM의 마지막 인스턴스 제거
    ISM->RemoveInstance(LastIndex);
}

void AVSEnemyManager::UpdateEnemies(float DeltaTime)
{
    // 플레이어 위치
    AVSCharacter* Player = Cast<AVSCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!Player) return;
    const FVector PlayerLoc = Player->GetActorLocation();

    const int32 Count = ISM->GetInstanceCount();
    TArray<FTransform> NewTransforms;
    NewTransforms.Reserve(Count);

    // 캐시 크기가 인스턴스 수와 다르면 스킵
    if (Enemies.Num() != Count)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mismatch: Locations=%d, Instances=%d"), Enemies.Num(), Count);
        return;
    }

    float ContactRangeSq = ContactRange * ContactRange;
    for (int32 i = 0; i < Count; ++i)
    {
        FVector& Loc = Enemies[i].Location;

        // 플레이어 방향으로 이동 (타입별 속도)
        const FVector Dir = (PlayerLoc - Loc).GetSafeNormal2D();
        Loc += Dir * Enemies[i].MoveSpeed * DeltaTime;

        // 이동 방향 바라보게 회전
        FRotator Rot = Dir.Rotation();
        Rot.Yaw -= 90.f;

        NewTransforms.Add(FTransform(Rot, Loc, FVector(Enemies[i].Scale)));

        // 플레이어 접촉 판정
        const float DistSq = FVector::DistSquared2D(Loc, PlayerLoc);
        if (DistSq < ContactRangeSq)
        {
            Player->TakeDamageFromEnemy(Enemies[i].ContactDamage * DeltaTime);  // 초당 대미지 (타입별)
        }
    }

    // 한 방에 갱신
    ISM->BatchUpdateInstancesTransforms(
        /*StartInstanceIndex*/ 0,
        NewTransforms,
        /*bWorldSpace*/ true,
        /*bMarkRenderStateDirty*/ true,
        /*bTeleport*/ true);
}

void AVSEnemyManager::ApplyDamageInRadius(const FVector& Center, float Radius, float Damage)
{
    const float RadiusSq = Radius * Radius;
    int32 HitCount = 0;
    TArray<int32> PendingKills;

    for (int32 i = 0; i < Enemies.Num(); ++i)
    {
        if (Enemies[i].Health <= 0.f) continue;
        const float DistSq = FVector::DistSquared2D(Center, Enemies[i].Location);
        if (DistSq <= RadiusSq)
        {
            Enemies[i].Health -= Damage;
            HitCount++;
            if (Enemies[i].Health <= 0.f)
                PendingKills.Add(i);
        }
    }

    PendingKills.Sort([](int32 A, int32 B) { return A > B; });
    for (int32 Idx : PendingKills)
        KillEnemy(Idx);
}