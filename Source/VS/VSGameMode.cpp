#include "VSGameMode.h"
#include "Character/VSPlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Manager/VSEnemyManager.h"
#include "Subsystem/VSDifficultySubsystem.h"

AVSGameMode::AVSGameMode()
{

}

void AVSGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (EnemyManagerClass)
	{
		GetWorld()->SpawnActor<AVSEnemyManager>(EnemyManagerClass);
	}

	if (UVSDifficultySubsystem* Diff = GetWorld()->GetSubsystem<UVSDifficultySubsystem>())
	{
		Diff->SetWaveData(WaveData);
	}
}