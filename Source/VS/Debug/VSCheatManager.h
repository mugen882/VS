#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "VSCheatManager.generated.h"

class AVSPlayerCharacter;
class AVSBenchmarkActor;

/**
 * VS 프로젝트 전용 콘솔 치트 모음.
 *
 * AVSPlayerController 생성자에서 CheatClass로 지정된다.
 *
 * 콘솔에서 사용:
 *      VSGiveAllWeapons 5  // 무기 전부 획득 + 각 무기 Lv.5까지 즉시 강화
 *      VSGiveAllPassive 5  // 패시브 전 종류를 Lv.5까지 즉시 강화
 *      VSAddXP 100         // 경험치 지급 (정상 레벨업 경로)
 *      VSSkipLevelUp 1     // 레벨업/업그레이드 봉인
 *      VSStopSpawn 1       // 웨이브 자동 스폰 전체 봉인 (VSSpawnBoss 수동 소환은 가능)
 *      VSBenchISM 300      // 벤치시작, 적소환(ISM + AnimToTexture)
 *      VSBenchActors 300   // 벤치시작, 적소환(액터 + 스켈레탈)
 *		VSSpawnGems 500     // 바닥에 XP 젬 500개 스폰
 *      VSEnemyClear        // 모든 적(잡몹·엘리트·미니언·보스) 즉시 제거 (+웨이브 스폰 봉인 여부)
 *      VSObjectClear       // VSEnemyClear + 바닥의 XP 젬까지 전부 제거
 *		VSSpawnBoss			// 보스 스폰. WaveIndex=-1이면 현재 웨이브, Distance=-1이면 기본 거리
 */
UCLASS()
class VS_API UVSCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	// --- 프로그레션 ---

	/**
	 * UpgradeComp->AllUpgrades에 등록된 NewWeapon 업그레이드의 무기를 전부 획득하고,
	 * 보유 중인 모든 무기를 TargetLevel까지 강화한다. 업그레이드 카드 UI는 뜨지 않는다.
	 */
	UFUNCTION(Exec)
	void VSGiveAllWeapons(int32 TargetLevel = 5);

	/**
	 * AllUpgrades에 등록된 Passive의 StatType을 전부 TargetLevel까지 올린다.
	 * 이미 그 이상 찍은 패시브는 그대로 둔다(스택은 되돌릴 수 없음).
	 */
	UFUNCTION(Exec)
	void VSGiveAllPassive(int32 TargetLevel = 5);

	/** 경험치 직접 지급. 필요 XP를 넘으면 평소와 동일하게 카드 UI가 뜬다. */
	UFUNCTION(Exec)
	void VSAddXP(int32 XP = 1000);

	/** 레벨업 처리 자체를 봉인/해제. 카드 UI가 안뜬다.*/
	UFUNCTION(Exec)
	void VSSkipLevelUp(bool bSkip = true);

	/** 웨이브 자동 스폰(잡몹·엘리트·보스) 봉인/해제. 시간 경과와 수동 보스 소환은 유지된다. */
	UFUNCTION(Exec)
	void VSStopSpawn(bool bStop = true);
	
	// --- 성능 측정 ---

	UFUNCTION(Exec)
	void VSBenchISM(int32 Count = 500);

	UFUNCTION(Exec)
	void VSBenchActors(int32 Count = 500);

	UFUNCTION(Exec)
	void VSSpawnGems(int32 Count = 500);

	/** 현재 살아있는 모든 적(ISM 잡몹·엘리트·미니언 + 보스)을 즉시 제거한다.
	* bSpawnDisable이 true이면 웨이브 자동 스폰도 봉인한다.
	*/
	UFUNCTION(Exec)
	void VSEnemyClear(bool bSpawnDisable = true);

	/** VSEnemyClear에 더해 바닥에 남은 XP 젬(픽업)까지 전부 제거한다. */
	UFUNCTION(Exec)
	void VSObjectClear(bool bSpawnDisable = true);

	UFUNCTION(Exec)
	void VSSpawnBoss(int32 WaveIndex = -1, float Distance = -1.f);

private:
	AVSPlayerCharacter* GetVSPlayerCharacter() const;
	AVSBenchmarkActor* FindBenchmarkActor() const;

	/** 치트 두 개가 공통으로 쓰는 UpgradeComponent 획득. 실패 시 로그 후 nullptr. */
	class UVSUpgradeComponent* GetUpgradeComponent(const TCHAR* CheatName) const;
};
