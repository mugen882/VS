#include "Enemy/VSBossSummoner.h"
#include "Data/VSBossData.h"
#include "Manager/VSEnemyManager.h"

void AVSBossSummoner::MoveTowardPlayer(float DeltaTime)
{
    const FVSBossPlayerInfo Info = QueryPlayer();
    if (!Info.IsValid()) return;

    ApplyKiting(Info, DeltaTime);
    FaceDirection(Info.Dir, GetRotateSpeedDeg(), DeltaTime);   // 항상 플레이어를 바라봄
    ApplyContactDamage(Info, DeltaTime);
}

void AVSBossSummoner::UpdateAttack(float DeltaTime)
{
    UVSBossSummonerData* CfgData = Cast<UVSBossSummonerData>(GetData());
    if (!CfgData || !CfgData->MinionType) return;

    SummonTimer -= DeltaTime;
    if (SummonTimer <= 0.f)
    {
        SummonMinions();
        SummonTimer = CfgData->SummonInterval;
    }
}

void AVSBossSummoner::SummonMinions()
{
    UVSBossSummonerData* CfgData = Cast<UVSBossSummonerData>(GetData());
    if (!CfgData) return;

    AVSEnemyManager* Mgr = EnemyManager.Get();
    if (!Mgr) return;

    const int32 Count = CfgData->MinionsPerSummon;
    if (Count <= 0) return;

    // 소환 기준 방향은 "플레이어 쪽". 선회가 느려도 소환 위치는 항상 플레이어와 보스 사이에 깔린다.
    const FVSBossPlayerInfo Info = QueryPlayer();
    const FVector Front = Info.IsValid() ? Info.Dir : GetActorForwardVector();

    const FVector SummonCenter = GetActorLocation();
    const float HalfDeg = CfgData->FrontSummonAngleHalfDeg;

    for (int32 i = 0; i < Count; ++i)
    {
        const float T = (Count == 1) ? 0.5f : (float)i / (Count - 1);
        const float OffsetDeg = FMath::Lerp(-HalfDeg, HalfDeg, T);
        const FVector Dir = Front.RotateAngleAxis(OffsetDeg, FVector::UpVector);

        FVector SpawnLoc = SummonCenter + Dir * CfgData->SummonDist;
        SpawnLoc.Z = 0.f;
        Mgr->SpawnEnemy(CfgData->MinionType, CfgData->MinionHealthMult, &SpawnLoc);
    }
}
