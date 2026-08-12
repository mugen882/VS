#include "Debug/VSStartupCommandSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"
#include "Common/VSLog.h"

bool UVSStartupCommandSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
#if UE_BUILD_SHIPPING
	// 출시 빌드에는 아예 존재하지 않는다
	return false;
#else
	return Super::ShouldCreateSubsystem(Outer);
#endif
}

void UVSStartupCommandSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (bExecuted || StartupCommands.Num() == 0) return;
	bExecuted = true;

	RemainingRetries = FMath::Max(0, StartupCommandRetryFrames);

	if (StartupCommandDelay <= 0.f)
	{
		InWorld.GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateUObject(this, &UVSStartupCommandSubsystem::RunStartupCommands));
	}
	else
	{
		InWorld.GetTimerManager().SetTimer(
			StartupTimerHandle,
			this,
			&UVSStartupCommandSubsystem::RunStartupCommands,
			StartupCommandDelay,
			false);
	}
}

void UVSStartupCommandSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StartupTimerHandle);
	}

	Super::Deinitialize();
}

void UVSStartupCommandSubsystem::RunStartupCommands()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// CheatManager는 PlayerController에 붙어 있다. 아직 없으면 다음 틱에 다시 본다.
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		RetryNextTick();
		return;
	}

	for (const FString& Cmd : StartupCommands)
	{
		const FString Trimmed = Cmd.TrimStartAndEnd();
		if (Trimmed.IsEmpty()) continue;

		UE_LOG(VSLog, Log, TEXT("StartupCommand: %s"), *Trimmed);
		PC->ConsoleCommand(Trimmed);
	}
}

void UVSStartupCommandSubsystem::RetryNextTick()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (RemainingRetries <= 0)
	{
		UE_LOG(VSLog, Warning,
			TEXT("StartupCommand: PlayerController를 %d프레임 동안 찾지 못해 실행을 포기합니다."),
			FMath::Max(0, StartupCommandRetryFrames));
		return;
	}

	--RemainingRetries;

	World->GetTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateUObject(this, &UVSStartupCommandSubsystem::RunStartupCommands));
}
