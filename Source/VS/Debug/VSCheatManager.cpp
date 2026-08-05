#include "Debug/VSCheatManager.h"
#include "Character/VSPlayerCharacter.h"
#include "Component/VSUpgradeComponent.h"
#include "Debug/VSBenchmarkActor.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Common/VSLog.h"
#include "Subsystem/VSDifficultySubsystem.h"

AVSPlayerCharacter* UVSCheatManager::GetVSPlayerCharacter() const
{
	APlayerController* PC = GetOuterAPlayerController();
	AVSPlayerCharacter* VSChar = PC ? Cast<AVSPlayerCharacter>(PC->GetPawn()) : nullptr;

	if (!VSChar)
	{
		UE_LOG(VSLog, Warning, TEXT("Cheat: 조종 중인 AVSPlayerCharacter가 없습니다."));
	}
	return VSChar;
}

UVSUpgradeComponent* UVSCheatManager::GetUpgradeComponent(const TCHAR* CheatName) const
{
	AVSPlayerCharacter* VSChar = GetVSPlayerCharacter();
	if (!VSChar) return nullptr;

	UVSUpgradeComponent* UpgradeComp = VSChar->GetUpgradeComponent();
	if (!UpgradeComp)
	{
		UE_LOG(VSLog, Warning, TEXT("%s: UVSUpgradeComponent를 찾을 수 없습니다."), CheatName);
	}
	return UpgradeComp;
}

AVSBenchmarkActor* UVSCheatManager::FindBenchmarkActor() const
{
	AVSBenchmarkActor* Bench = Cast<AVSBenchmarkActor>(
		UGameplayStatics::GetActorOfClass(this, AVSBenchmarkActor::StaticClass()));

	if (!Bench)
	{
		UE_LOG(VSLog, Warning, TEXT("VSBench: 레벨에 AVSBenchmarkActor가 없습니다."));
	}
	return Bench;
}

// --- 프로그레션 ---

void UVSCheatManager::VSGiveAllWeapons(int32 TargetLevel)
{
	if (UVSUpgradeComponent* UpgradeComp = GetUpgradeComponent(TEXT("VSGiveAllWeapons")))
	{
		UpgradeComp->CheatGiveAllWeapons(TargetLevel);
	}
}

void UVSCheatManager::VSGiveAllPassive(int32 TargetLevel)
{
	if (UVSUpgradeComponent* UpgradeComp = GetUpgradeComponent(TEXT("VSGiveAllPassive")))
	{
		UpgradeComp->CheatGiveAllPassives(TargetLevel);
	}
}

void UVSCheatManager::VSAddXP(int32 XP)
{
	if (AVSPlayerCharacter* VSChar = GetVSPlayerCharacter())
	{
		VSChar->AddXP(XP);
	}
}

void UVSCheatManager::VSSkipLevelUp(bool bSkip)
{
	if (AVSPlayerCharacter* VSChar = GetVSPlayerCharacter())
	{
		VSChar->SkipLevelUp(bSkip);
		UE_LOG(VSLog, Warning, TEXT("VSSkipLevelUp: %s"), bSkip ? TEXT("ON") : TEXT("OFF"));
	}
}

// --- 성능 측정 ---

void UVSCheatManager::VSBench(int32 Count)
{
	if (AVSBenchmarkActor* Bench = FindBenchmarkActor())
	{
		Bench->RunBenchmark(EVSBenchMode::ISM, Count);
	}
}

void UVSCheatManager::VSBenchActor(int32 Count)
{
	if (AVSBenchmarkActor* Bench = FindBenchmarkActor())
	{
		Bench->RunBenchmark(EVSBenchMode::Actors, Count);
	}
}

void UVSCheatManager::VSBenchClear()
{
	if (AVSBenchmarkActor* Bench = FindBenchmarkActor())
	{
		Bench->ClearAll();
	}
}

void UVSCheatManager::VSStopSpawn(bool bStop)
{
	UWorld* World = GetWorld();
	if (!World) return;

	UVSDifficultySubsystem* Diff = World->GetSubsystem<UVSDifficultySubsystem>();
	if (!Diff)
	{
		UE_LOG(VSLog, Warning, TEXT("VSStopSpawn: UVSDifficultySubsystem을 찾지 못했습니다."));
		return;
	}

	Diff->SetWaveSpawnDisabled(bStop);
	UE_LOG(VSLog, Warning, TEXT("VSStopSpawn: %s (VSSpawnBoss 수동 소환은 가능)"), bStop ? TEXT("ON") : TEXT("OFF"));
}

void UVSCheatManager::VSSpawnBoss(int32 WaveIndex, float Distance)
{
	UWorld* World = GetWorld();
	if (!World) return;

	UVSDifficultySubsystem* Diff = World->GetSubsystem<UVSDifficultySubsystem>();
	if (!Diff)
	{
		UE_LOG(VSLog, Warning, TEXT("VSSpawnBoss: UVSDifficultySubsystem을 찾지 못했습니다."));
		return;
	}

	if (Diff->SpawnBossNow(WaveIndex, Distance))
	{
		UE_LOG(VSLog, Warning, TEXT("VSSpawnBoss: 소환 완료 (WaveIndex=%d, Distance=%.0f)"), WaveIndex, Distance);
	}
	else
	{
		UE_LOG(VSLog, Warning, TEXT("VSSpawnBoss: 소환 실패. 해당 웨이브에 BossClass가 지정되어 있는지 확인하세요."));
	}
}