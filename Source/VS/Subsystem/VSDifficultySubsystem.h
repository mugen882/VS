#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VSDifficultySubsystem.generated.h"

class UVSWaveData;

UCLASS()
class UVSDifficultySubsystem : public UWorldSubsystem, public FTickableGameObject
{
    GENERATED_BODY()
public:
    float GetElapsedTime() const { return ElapsedTime; }

protected:
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;

private:
    float ElapsedTime = 0.f;
    int32 CurrentWaveIndex = 0;
    float SpawnAccumulator = 0.f;

    UPROPERTY()
    TObjectPtr<UVSWaveData> WaveData;   // 웨이브 정의
};