#include "VSGameMode.h"
#include "Character/VSPlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Manager/VSEnemyManager.h"
#include "Subsystem/VSDifficultySubsystem.h"

AVSGameMode::AVSGameMode()
{

}

void AVSGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	if (EnemyManagerClass)
	{
		EnemyManager = GetOrCreateEnemyManager();
	}
}

void AVSGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UVSDifficultySubsystem* Diff = GetWorld()->GetSubsystem<UVSDifficultySubsystem>())
	{
		Diff->SetWaveData(WaveData);
	}
}

void AVSGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (EnemyManager)
	{
		EnemyManager->Destroy();
		EnemyManager = nullptr;
	}
}

AVSEnemyManager* AVSGameMode::GetOrCreateEnemyManager()
{
	if (!IsValid(EnemyManager) && EnemyManagerClass)
	{
		EnemyManager = GetWorld()->SpawnActor<AVSEnemyManager>(EnemyManagerClass);
	}
	return EnemyManager;
}