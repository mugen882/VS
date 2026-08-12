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
	if (!Super::ShouldCreateSubsystem(Outer)) return false;

	// 에디터 프리뷰·썸네일 월드 등에서는 만들지 않는다
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
#endif
}

void UVSStartupCommandSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	if (bExecuted || StartupCommands.Num() == 0) return;
	bExecuted = true;

	if (StartupCommandDelay <= 0.f)
	{
		// 여기는 모든 BeginPlay 이후이자 첫 Tick 이전이라 그대로 실행해도 안전하다
		RunStartupCommands();
		return;
	}

	FTimerHandle Handle;
	InWorld.GetTimerManager().SetTimer(
		Handle,
		this,
		&UVSStartupCommandSubsystem::RunStartupCommands,
		StartupCommandDelay,
		false);
}

void UVSStartupCommandSubsystem::RunStartupCommands()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 콘솔 명령 실행은 PlayerController를 거친다 (치트 명령이 여기 체인에 붙는다)
	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(VSLog, Warning, TEXT("StartupCommand: PlayerController가 없어 건너뜁니다."));
		return;
	}

	for (const FString& Cmd : StartupCommands)
	{
		const FString Trimmed = Cmd.TrimStartAndEnd();
		if (Trimmed.IsEmpty()) continue;

		UE_LOG(VSLog, Warning, TEXT("StartupCommand: %s"), *Trimmed);
		PC->ConsoleCommand(Trimmed);
	}
}
