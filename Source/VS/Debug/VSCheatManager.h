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
 *      VSSkipLevelUp 1     // 레벨업/업그레이드 UI 봉인 (벤치마크용)
 *      VSBench 300         // ISM + AnimToTexture
 *      VSBenchActor 300    // 액터 + 스켈레탈
 *      VSBenchClear        // 벤치마크 정리
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
	void VSAddXP(int32 XP);

	/** 레벨업 처리 자체를 봉인/해제. 카드 UI가 안뜬다.*/
	UFUNCTION(Exec)
	void VSSkipLevelUp(bool bSkip);

	// --- 성능 측정 ---

	UFUNCTION(Exec)
	void VSBench(int32 Count);

	UFUNCTION(Exec)
	void VSBenchActor(int32 Count);

	UFUNCTION(Exec)
	void VSBenchClear();

private:
	AVSPlayerCharacter* GetVSPlayerCharacter() const;
	AVSBenchmarkActor* FindBenchmarkActor() const;

	/** 치트 두 개가 공통으로 쓰는 UpgradeComponent 획득. 실패 시 로그 후 nullptr. */
	class UVSUpgradeComponent* GetUpgradeComponent(const TCHAR* CheatName) const;
};
