#include "Subsystem/VSDifficultySubsystem.h"

void UVSDifficultySubsystem::Tick(float DeltaTime)
{
	ElapsedTime += DeltaTime;
}

TStatId UVSDifficultySubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UVSDifficultySubsystem, STATGROUP_Tickables);
}

bool UVSDifficultySubsystem::IsTickable() const
{
	return !IsTemplate();   // CDO(클래스 기본 객체)는 틱 안 하게
}