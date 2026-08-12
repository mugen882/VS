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

	AVSEnemyManager* GetOrCreateEnemyManager();

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AVSEnemyManager> EnemyManagerClass;

	UPROPERTY(EditAnywhere, Category="Wave")
	TObjectPtr<UVSWaveData> WaveData;

protected:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	TObjectPtr<AVSEnemyManager> EnemyManager;
};