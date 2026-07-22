// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Data/VSWaveData.h"
#include "Blueprint/UserWidget.h"
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

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> ResultWidgetClass;

protected:
	virtual void BeginPlay() override;

private:
	void HandleRunCleared();
};



