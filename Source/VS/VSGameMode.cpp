#include "VSGameMode.h"
#include "Character/VSPlayerController.h"
#include "Character/VSPlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Manager/VSEnemyManager.h"
#include "Subsystem/VSDifficultySubsystem.h"
#include "Kismet/GameplayStatics.h"

AVSGameMode::AVSGameMode()
{
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Characters/Blueprints/BP_PlayerCharacter"));
	//if (PlayerPawnBPClass.Class != nullptr)
	//{
	//	DefaultPawnClass = PlayerPawnBPClass.Class;
	//}

	//PlayerControllerClass = AVSPlayerController::StaticClass();

	//static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/Characters/Blueprints/BP_PlayerController"));
	//if (PlayerControllerBPClass.Succeeded())
	//{
	//	PlayerControllerClass = PlayerControllerBPClass.Class;
	//}
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
		Diff->OnRunCleared.AddUObject(this, &AVSGameMode::HandleRunCleared);
	}
}

void AVSGameMode::HandleRunCleared()
{
	if (AVSPlayerController* PC = Cast<AVSPlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		PC->ShowResult(/*bIsVictory=*/true);
}