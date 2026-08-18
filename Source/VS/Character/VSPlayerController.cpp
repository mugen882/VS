#include "VSPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Character/VSPlayerCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Subsystem/VSDifficultySubsystem.h"
#include "UI/VSResultWidget.h"
#include "UI/VSHUDWidget.h"
#include "ViewModel/VSHUDViewModel.h"
#include "Debug/VSCheatManager.h"
#include "Common/VSLog.h"
#include "UI/VSPauseMenuWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AVSPlayerController::AVSPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	CheatClass = UVSCheatManager::StaticClass();
}

void AVSPlayerController::AddCheats(bool bForce)
{
#if UE_BUILD_SHIPPING
	Super::AddCheats(bForce);
#else
	Super::AddCheats(true);
#endif
}

void AVSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UWorld* World = GetWorld())
	{
		if (UVSDifficultySubsystem* Diff = World->GetSubsystem<UVSDifficultySubsystem>())
		{
			Diff->OnRunCleared.AddUObject(this, &AVSPlayerController::HandleRunCleared);
		}
	}
}

void AVSPlayerController::Move(const FInputActionValue& Value)
{
	AVSPlayerCharacter* VSChar = Cast<AVSPlayerCharacter>(GetPawn());
	if (VSChar == nullptr) return;

	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator YawRotation(0, GetControlRotation().Yaw, 0);
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	VSChar->AddMovementInput(ForwardDir, MovementVector.Y);
	VSChar->AddMovementInput(RightDir, MovementVector.X);
}

void AVSPlayerController::HandlePlayerDied()
{
	ShowResult(/*bIsVictory=*/false);
}

void AVSPlayerController::ShowResult(bool bIsVictory)
{
	if (ResultWidgetClass.IsNull())
	{
		UE_LOG(VSLog, Error, TEXT("ShowResult: ResultWidgetClass가 지정되지 않음"));
		return;
	}

	// 결과 수집
	float Survival = 0.f;
	int32 Kills = 0;
	int32 Wave = 1;
	if (UWorld* World = GetWorld())
	{
		if (UVSDifficultySubsystem* Diff = World->GetSubsystem<UVSDifficultySubsystem>())
		{
			Survival = Diff->GetElapsedTime();
			Kills = Diff->GetKillCount();
			Wave = Diff->GetCurrentWaveNumber();
		}
	}

	int32 Level = 1;
	if (AVSPlayerCharacter* PC = Cast<AVSPlayerCharacter>(GetPawn()))
		Level = PC->CurrentLevel;

	// 결과 위젯 생성 + 결과 전달 (승/패 구분)
	if (UClass* LoadedResultWidgetClass = ResultWidgetClass.LoadSynchronous())
	{
		if (UVSResultWidget* Widget = CreateWidget<UVSResultWidget>(this, LoadedResultWidgetClass))
		{
			Widget->AddToViewport();
			Widget->SetupResult(bIsVictory, Survival, Kills, Level, Wave);
		}
	}

	bGameOver = true;	// 결과 화면 이후 ESC 메뉴 차단
	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());	// UI만 입력받기
	SetPause(true);	// 일시정지
}

void AVSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (auto* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (MoveAction)
			EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AVSPlayerController::Move);

		if (PauseAction)
		{
			EIC->BindAction(PauseAction, ETriggerEvent::Started, this, &AVSPlayerController::TogglePauseMenu);
		}
	}
}

void AVSPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (auto* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->ClearAllMappings();
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	if (AVSPlayerCharacter* PC = Cast<AVSPlayerCharacter>(GetPawn()))
		PlayerDiedHandle = PC->OnPlayerDied.AddUObject(this, &AVSPlayerController::HandlePlayerDied);

	SetupHUD();
}

void AVSPlayerController::OnUnPossess()
{
	if (AVSPlayerCharacter* PC = Cast<AVSPlayerCharacter>(GetPawn()))
		PC->OnPlayerDied.Remove(PlayerDiedHandle);

	PlayerDiedHandle.Reset();

	TeardownHUD();

	Super::OnUnPossess();
}

void AVSPlayerController::SetupHUD()
{
	AVSPlayerCharacter* PC = Cast<AVSPlayerCharacter>(GetPawn());
	if (!PC || HUDWidgetClass.IsNull()) return;

	UVSDifficultySubsystem* Diff = nullptr;
	if (UWorld* World = GetWorld())
		Diff = World->GetSubsystem<UVSDifficultySubsystem>();

	HUDViewModel = NewObject<UVSHUDViewModel>(this);
	HUDViewModel->BindModels(PC, Diff);

	if (UClass* LoadedHUDWidgetClass = HUDWidgetClass.LoadSynchronous())
	{
		HUDWidget = CreateWidget<UVSHUDWidget>(this, LoadedHUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->SetViewModel(HUDViewModel);
			HUDWidget->AddToViewport();
		}
	}
}

void AVSPlayerController::TeardownHUD()
{
	if (HUDWidget)
	{
		HUDWidget->RemoveFromParent();
		HUDWidget = nullptr;
	}

	HUDViewModel = nullptr;
}

void AVSPlayerController::HandleRunCleared()
{
	ShowResult(/*bIsVictory=*/true);
}

void AVSPlayerController::TogglePauseMenu()
{
	if (bGameOver)
	{
		return;   // 결과 화면 위에 메뉴가 겹치지 않게
	}

	if (PauseMenuWidget)
	{
		if (PauseMenuWidget->HandleBackPressed())
		{
			return;   // 종료 팝업만 닫힘, 메뉴는 유지
		}
		ClosePauseMenu();
		return;
	}

	// 레벨업 업그레이드 선택 등 다른 UI가 이미 멈춰둔 상태면 개입하지 않는다
	if (UGameplayStatics::IsGamePaused(this))
	{
		return;
	}

	OpenPauseMenu();
}

void AVSPlayerController::OpenPauseMenu()
{
	if (PauseMenuWidget || !PauseMenuWidgetClass)
	{
		return;
	}

	PauseMenuWidget = CreateWidget<UVSPauseMenuWidget>(this, PauseMenuWidgetClass);
	if (!PauseMenuWidget)
	{
		return;
	}

	PauseMenuWidget->AddToViewport(10);

	// 게임 중 커서 상태를 기억했다가 닫을 때 그대로 되돌린다
	bCursorVisibleBeforePause = bShowMouseCursor;
	bShowMouseCursor = true;

	FInputModeUIOnly Mode;
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);

	PauseMenuWidget->FocusMenu();

	if (UWorld* World = GetWorld())
	{
		if (UVSDifficultySubsystem* Diff = World->GetSubsystem<UVSDifficultySubsystem>())
		{
			Diff->SetPauseGame(true);
		}
	}

	SetPause(true);
}

void AVSPlayerController::ClosePauseMenu()
{
	if (PauseMenuWidget)
	{
		PauseMenuWidget->RemoveFromParent();
		PauseMenuWidget = nullptr;
	}

	bShowMouseCursor = bCursorVisibleBeforePause;
	SetInputMode(FInputModeGameOnly());

	if (UWorld* World = GetWorld())
	{
		if (UVSDifficultySubsystem* Diff = World->GetSubsystem<UVSDifficultySubsystem>())
		{
			Diff->SetPauseGame(false);
		}
	}

	SetPause(false);	// 일시정지 해제
}

void AVSPlayerController::RestartGame()
{
	ClosePauseMenu();   // 일시정지 해제가 OpenLevel보다 먼저여야 한다

	const FName CurrentLevel = FName(*UGameplayStatics::GetCurrentLevelName(this, true));
	UGameplayStatics::OpenLevel(this, CurrentLevel);
}

void AVSPlayerController::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}