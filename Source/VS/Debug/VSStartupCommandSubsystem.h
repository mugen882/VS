#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VSStartupCommandSubsystem.generated.h"

/**
 * 게임 시작 직후 지정한 콘솔 명령을 순서대로 실행하는 디버그용 서브시스템.
 *
 * 명령 목록은 DefaultGame.ini에서 관리한다.
 *      [/Script/VS.VSStartupCommandSubsystem]
 *      +StartupCommands=VSGiveAllWeapons 5
 *      +StartupCommands=VSStopSpawn 1
 *
 * 실행 시점은 OnWorldBeginPlay — 모든 액터의 BeginPlay가 끝난 직후이자 첫 Tick 전이다.
 * 따라서 폰 빙의·매니저 스폰이 이미 완료돼 있고, 웨이브 자동 스폰은 아직 시작되지 않았다.
 *
 * 다만 CheatManager는 PlayerController가 준비된 뒤에야 붙는다. PlayerController가
 * 아직 없으면 StartupCommandRetryFrames 만큼 다음 틱에 재시도한다.
 *
 * Shipping 빌드에서는 서브시스템 자체가 생성되지 않는다.
 */
UCLASS(Config = Game)
class VS_API UVSStartupCommandSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

private:
	/** StartupCommands를 순서대로 실행한다. PlayerController가 없으면 재시도를 예약한다. */
	void RunStartupCommands();

	/** 다음 틱에 RunStartupCommands를 다시 시도한다. 남은 횟수가 없으면 포기하고 경고한다. */
	void RetryNextTick();

private:
	/**
	 * 시작 직후 실행할 콘솔 명령 목록. 위에서부터 순서대로 실행된다.
	 * ini에서 '+' 로 추가할 것 ('=' 만 쓰면 마지막 한 줄만 남는다).
	 */
	UPROPERTY(Config)
	TArray<FString> StartupCommands;

	/**
	 * 실행을 늦출 시간(초). 0이면 OnWorldBeginPlay에서 즉시 실행한다.
	 * 첫 Tick 이후에나 준비되는 대상을 건드려야 할 때만 값을 준다.
	 */
	UPROPERTY(Config)
	float StartupCommandDelay = 0.f;

	/** PlayerController를 기다리며 재시도할 최대 프레임 수. */
	UPROPERTY(Config)
	int32 StartupCommandRetryFrames = 10;

	/** 지연·재시도 예약 핸들. Deinitialize에서 정리한다. */
	FTimerHandle StartupTimerHandle;

	/** 남은 재시도 횟수. */
	int32 RemainingRetries = 0;

	/** 월드당 1회 보장. */
	bool bExecuted = false;
};
