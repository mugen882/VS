#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Common/VSDefine.h"
#include "VSBenchmarkDummy.generated.h"

class USkeletalMeshComponent;
class APawn;

/**
 * 성능 비교용 "간단한" 적 구현.
 *
 * ISM + AnimToTexture 경로와 대조하기 위한 베이스라인이다.
 * 개체마다 액터 + 스켈레탈 메시 컴포넌트를 갖고 매 틱 플레이어를 향해 이동한다
 *  — AVSEnemyManager가 배열 하나로 처리하는 일을 액터 N개로 나눠서 하는 셈.
 *
 * 공정한 비교를 위해 잡몹과 같은 일만 한다: 플레이어 추적 이동 + 회전.
 * (데미지·젬 드롭 등은 렌더링 비용 비교에 영향이 없어 제외)
 *
 * 사용법: 이 클래스를 상속한 BP를 만들어 메시와 애님 BP를 지정한 뒤,
 * AVSBenchmarkActor의 DummyClass에 지정한다.
 */
UCLASS()
class VS_API AVSBenchmarkDummy : public AActor
{
    GENERATED_BODY()

public:
    AVSBenchmarkDummy();

    virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USkeletalMeshComponent> MeshComp;

    UPROPERTY(EditAnywhere, Category = "Benchmark")
    float MoveSpeed = 200.f;

    // 액터 정면(+X) 대비 메시가 틀어진 각도. 잡몹/보스와 같은 규칙
    UPROPERTY(EditAnywhere, Category = "Benchmark", meta = (ClampMin = "-180", ClampMax = "180"))
    float MeshYawOffset = MESH_YAW_OFFSET;

private:
    // 매 틱 GetPlayerPawn을 다시 부르지 않도록 캐시(ISM 경로는 프레임당 1회만 조회한다).
    TWeakObjectPtr<APawn> CachedPlayer;
};
