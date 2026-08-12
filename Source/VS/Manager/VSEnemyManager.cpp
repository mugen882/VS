#include "VSEnemyManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "VSGemManager.h"
#include "Character/VSPlayerCharacter.h"
#include "Data/VSEnemyTypeData.h"
#include "Enemy/VSBossEnemy.h"
#include "Subsystem/VSDifficultySubsystem.h"
#include "Common/VSLog.h"

AVSEnemyManager::AVSEnemyManager()
{
    PrimaryActorTick.bCanEverTick = true;

    ISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ISM"));
    ISM->SetReceivesDecals(false);   // 바닥 데칼이 잡몹 위로 투영되지 않게
    RootComponent = ISM;
    ISM->NumCustomDataFloats = 7; // 인스턴스당 커스텀 데이터
    ISM->SetCastShadow(false);
}

void AVSEnemyManager::BeginPlay()
{
    Super::BeginPlay();

    if (ISM)
        ISM->ClearInstances();
    Enemies.Empty();

    if (AVSPlayerCharacter* Player = Cast<AVSPlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0)))
    {
        GemManager = Player->GetGemManager();
    }
}

void AVSEnemyManager::SpawnEnemy(const UVSEnemyTypeData* Type, float HealthMult, const FVector* MinionLoc)
{
    if (!Type || !ISM) return;
    if (Enemies.Num() >= MaxEnemies) return;

    FVector Loc = FVector::ZeroVector;
    if (MinionLoc)
    {
        Loc = *MinionLoc;
    }
    else
    {
        APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
        FVector SpawnCenter = Player ? Player->GetActorLocation() : GetActorLocation();
        // 링 영역에 스폰: 랜덤 각도 + 랜덤 거리(Min~Max)
        float Dist = FMath::FRandRange(MinSpawnRadius, MaxSpawnRadius);
        float Angle = FMath::FRandRange(0.f, 2.f * PI);
        Loc = FVector(SpawnCenter.X + FMath::Cos(Angle) * Dist,
            SpawnCenter.Y + FMath::Sin(Angle) * Dist,
            0.f);
    }

    // 타입 크기 반영
    float Scale = FMath::Min(Type->Scale, MAX_ENEMY_SCALE);
    const FTransform Xform(FRotator::ZeroRotator, Loc, FVector(Scale));
    const int32 Index = ISM->AddInstance(Xform, /*bWorldSpace=*/true);

    // 애니메이션용 커스텀 데이터
    ISM->SetCustomDataValue(Index, (int32)EEnemyCustomData::Anim_StartTime, FMath::FRand());
    ISM->SetCustomDataValue(Index, (int32)EEnemyCustomData::Anim_Speed_Rate, 1.0f);
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

    // 보스도 후보에 포함 (조준용 위치). 보스가 더 가까우면 위치를 덮되,
    // 유효 타겟 신호를 위해 BestIndex를 BOSS_TARGET_INDEX로 표시
    float BossDistSq;
    if (AVSBossEnemy* Boss = FindNearestBoss(From, MaxRange, BossDistSq))
    {
        if (BossDistSq < BestDistSq)
        {
            OutLocation = Boss->GetActorLocation();
            BestIndex = BOSS_TARGET_INDEX;
        }
    }
    return BestIndex;
}

void AVSEnemyManager::RegisterBoss(AVSBossEnemy* Boss)
{
    if (Boss)
        Bosses.AddUnique(Boss);
}

void AVSEnemyManager::UnregisterBoss(AVSBossEnemy* Boss)
{
    Bosses.Remove(Boss);
}

AVSBossEnemy* AVSEnemyManager::FindNearestBoss(const FVector& From, float MaxRange, float& OutDistSq) const
{
    AVSBossEnemy* Best = nullptr;
    float BestDistSq = MaxRange * MaxRange;

    for (const TObjectPtr<AVSBossEnemy>& Boss : Bosses)
    {
        if (!IsValid(Boss)) continue;
        const float DistSq = FVector::DistSquared2D(From, Boss->GetActorLocation());
        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            Best = Boss;
        }
    }
    OutDistSq = BestDistSq;
    return Best;
}

void AVSEnemyManager::ApplyDamageToEnemy(int32 Index, float Damage)
{
    if (!Enemies.IsValidIndex(Index)) return;
    if (Enemies[Index].Health <= 0.f) return;

    Enemies[Index].Health -= Damage;
    if (Enemies[Index].Health <= 0.f)
    {
        KillEnemy(Index);
    }
}

void AVSEnemyManager::ClearAllEnemies()
{
	// 일반, 엘리트, 미니언 등 모든 적 제거.
    Enemies.Reset();
    if (ISM)
    {
        ISM->ClearInstances();
    }

    TArray<TObjectPtr<AVSBossEnemy>> BossesCopy = Bosses;
    for (const TObjectPtr<AVSBossEnemy>& Boss : BossesCopy)
    {
        if (IsValid(Boss))
        {
            Boss->Kill(false);   // 일괄 정리이므로 드랍은 생략
        }
    }
    Bosses.Reset();
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

    Enemies[Index] = Enemies[LastIndex];
    Enemies.RemoveAt(LastIndex);

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

    ISM->RemoveInstance(LastIndex);
}

void AVSEnemyManager::UpdateEnemies(float DeltaTime)
{
    AVSPlayerCharacter* Player = Cast<AVSPlayerCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!Player) return;
    const FVector PlayerLoc = Player->GetActorLocation();

    const int32 Count = ISM->GetInstanceCount();
    TArray<FTransform> NewTransforms;
    NewTransforms.Reserve(Count);

    if (Enemies.Num() != Count)
    {
        UE_LOG(VSLog, Warning, TEXT("Mismatch: Locations=%d, Instances=%d"), Enemies.Num(), Count);
        return;
    }

    float ContactRangeSq = ContactRange * ContactRange;
    for (int32 i = 0; i < Count; ++i)
    {
        FVector& Loc = Enemies[i].Location;

        // 플레이어 방향
        const FVector ToPlayer = PlayerLoc - Loc;
        const FVector Dir = ToPlayer.GetSafeNormal2D();
        const float DistToPlayer = ToPlayer.Size2D();

        // 너무 가까우면 이동을 멈춘다
        // 접촉 사거리 안이면 어차피 접촉 대미지가 들어가므로 더 파고들 필요가 없다.
        if (DistToPlayer > ContactRange)
        {
            Loc += Dir * Enemies[i].MoveSpeed * DeltaTime;
        }

        // 회전은 항상 플레이어를 바라보게
        FRotator Rot = Dir.Rotation();
        Rot.Yaw += MeshYawOffset;

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
        /*bMarkRenderStateDirty*/ false,
        /*bTeleport*/ true);
}

void AVSEnemyManager::ApplyDamageInRadius(const FVector& Center, float Radius, float Damage)
{
    const float RadiusSq = Radius * Radius;
    TArray<int32> PendingKills;

    for (int32 i = 0; i < Enemies.Num(); ++i)
    {
        if (Enemies[i].Health <= 0.f) continue;
        const float DistSq = FVector::DistSquared2D(Center, Enemies[i].Location);
        if (DistSq <= RadiusSq)
        {
            Enemies[i].Health -= Damage;
            if (Enemies[i].Health <= 0.f)
                PendingKills.Add(i);
        }
    }

    PendingKills.Sort([](int32 A, int32 B) { return A > B; });
    for (int32 Idx : PendingKills)
        KillEnemy(Idx);

    // 범위 내 보스 타격
    // 바로 데미지 적용해서 사망처리하면 삭제되면서 Ensure 발생할 수 있어서
    // 대상 수집 후 별도 루프에서 타격
    TArray<AVSBossEnemy*, TInlineAllocator<4>> BossTargets;
    for (const TObjectPtr<AVSBossEnemy>& Boss : Bosses)
    {
        if (!IsValid(Boss)) continue;
        if (FVector::DistSquared2D(Center, Boss->GetActorLocation()) <= RadiusSq)
        {
            // 대상을 수집
            BossTargets.Add(Boss);
        }   
    }

    // 보스 타격
    for (AVSBossEnemy* Boss : BossTargets)
    {
        if (!IsValid(Boss)) continue;
        Boss->ReceiveDamage(Damage);
    }
}