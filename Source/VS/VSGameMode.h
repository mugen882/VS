#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Data/VSWaveData.h"
#include "VSGameMode.generated.h"

class AVSEnemyManager;

UCLASS(minimalapi)
class AVSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AVSGameMode();

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AVSEnemyManager> EnemyManagerClass;

	UPROPERTY(EditAnywhere, Category="Wave")
	TObjectPtr<UVSWaveData> WaveData;

	UPROPERTY(EditAnywhere, Category = "Wave")
	bool bDisableWaveSpawn = false;

protected:
	virtual void BeginPlay() override;
};