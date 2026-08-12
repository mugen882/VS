#include "Debug/VSCheatManager.h"
#include "Character/VSPlayerCharacter.h"
#include "Component/VSUpgradeComponent.h"
#include "Debug/VSBenchmarkActor.h"
#include "Manager/VSEnemyManager.h"
#include "Manager/VSGemManager.h"
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
		UE_LOG(VSLog, Warning, TEXT("Cheat: AVSPlayerCharacter가 없습니다."));
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

void UVSCheatManager::VSStopXP(bool bStop)
{
	if (AVSPlayerCharacter* VSChar = GetVSPlayerCharacter())
	{
		VSChar->StopXP(bStop);
		UE_LOG(VSLog, Warning, TEXT("VSStopXP: %s"), bStop ? TEXT("ON") : TEXT("OFF"));
	}
}

void UVSCheatManager::VSBenchISM(int32 Count)
{
	if (AVSBenchmarkActor* Bench = FindBenchmarkActor())
	{
		Bench->RunBenchmark(EVSBenchMode::ISM, Count);
	}
}

void UVSCheatManager::VSBenchActors(int32 Count)
{
	if (AVSBenchmarkActor* Bench = FindBenchmarkActor())
	{
		Bench->RunBenchmark(EVSBenchMode::Actors, Count);
	}
}


void UVSCheatManager::VSSpawnGems(int32 Count)
{
	AVSPlayerCharacter* Player = GetVSPlayerCharacter();
	if (!Player)
	{
		UE_LOG(VSLog, Warning, TEXT("VSSpawnGems: Player를 찾을 수 없습니다."));
		return;
	}

	AVSGemManager* GemManager = Player->GetGemManager();
	if (!GemManager)
	{
		UE_LOG(VSLog, Warning, TEXT("VSSpawnGems: GemManager를 찾을 수 없습니다."));
		return;
	}

	FVector Center = Player->GetActorLocation();
	Center.Z -= Player->GetSimpleCollisionHalfHeight();
	for (int32 i = 0; i < Count; ++i)
	{
		const float Angle = FMath::FRandRange(0.f, 360.f);
		const float Dist = FMath::FRandRange(MIN_SPAWN_RADIUS, MAX_SPAWN_RADIUS);
		GemManager->SpawnGem(Center + FVector(Dist, 0.f, 0.f).RotateAngleAxis(Angle, FVector::UpVector), 1);
	}
}

void UVSCheatManager::VSEnemyClear()
{
	AVSEnemyManager* Mgr = Cast<AVSEnemyManager>(
		UGameplayStatics::GetActorOfClass(this, AVSEnemyManager::StaticClass()));
	if (Mgr)
	{
		Mgr->ClearAllEnemies();
	}
	else
	{
		UE_LOG(VSLog, Warning, TEXT("VSEnemyClear: 레벨에 AVSEnemyManager가 없습니다."));
	}

	// 벤치마크 액터 경로로 스폰된 더미 액터도 함께 제거
	if (AVSBenchmarkActor* Bench = FindBenchmarkActor())
	{
		Bench->ClearDummies();
	}
}

void UVSCheatManager::VSObjectClear()
{
	VSEnemyClear();

	AVSPlayerCharacter* Player = GetVSPlayerCharacter();
	if (!Player)
	{
		UE_LOG(VSLog, Warning, TEXT("VSObjectClear: Player를 찾을 수 없습니다."));
		return;
	}

	AVSGemManager* GemManager = Player->GetGemManager();
	if (!GemManager)
	{
		UE_LOG(VSLog, Warning, TEXT("VSObjectClear: AVSGemManager를 찾을 수 없습니다."));
		return;
	}

	// 바닥에 남은 XP 젬도 제거
	GemManager->ClearAllGems();
}

void UVSCheatManager::VSStopSpawn(bool bStop)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(VSLog, Warning, TEXT("VSStopSpawn: World를 찾을 수 없습니다."));
		return;
	}

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
	if (!World)
	{
		UE_LOG(VSLog, Warning, TEXT("VSSpawnBoss: World를 찾을 수 없습니다."));
		return;
	}

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