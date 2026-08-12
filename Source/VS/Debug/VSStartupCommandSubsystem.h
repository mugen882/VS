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
 * Shipping 빌드에서는 서브시스템 자체가 생성되지 않는다.
 */
UCLASS(Config = Game)
class UVSStartupCommandSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
	/** StartupCommands를 순서대로 실행한다. */
	void RunStartupCommands();

private:
	/**
	 * 시작 직후 실행할 콘솔 명령 목록. 위에서부터 순서대로 실행된다.
	 * ini에서 '+' 로 추가할 것 ('=' 만 쓰면 마지막 한 줄만 남는다).
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Startup")
	TArray<FString> StartupCommands;

	/**
	 * 실행을 늦출 시간(초). 0이면 OnWorldBeginPlay에서 즉시 실행한다.
	 * 첫 Tick 이후에나 준비되는 대상을 건드려야 할 때만 값을 준다.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Startup")
	float StartupCommandDelay = 0.f;

	/** 월드당 1회 보장. */
	bool bExecuted = false;
};
