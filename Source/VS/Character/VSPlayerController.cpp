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

AVSPlayerController::AVSPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void AVSPlayerController::BeginPlay()
{
	Super::BeginPlay();
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
	// 결과 수집: 생존 시간·처치 수·도달 웨이브 (서브시스템) + 레벨 (캐릭터)
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

	// 결과 위젯 생성 + 결과 전달 (승/패 구분) + 화면에 추가
	if (ResultWidgetClass)
	{
		if (UClass* LoadedResultWidgetClass = ResultWidgetClass.LoadSynchronous())
		{
			if (UVSResultWidget* Widget = CreateWidget<UVSResultWidget>(this, LoadedResultWidgetClass))
			{
				Widget->AddToViewport();
				Widget->SetupResult(bIsVictory, Survival, Kills, Level, Wave);
			}
		}
	}

	// 입력을 UI로 전환하고 게임 일시정지
	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());
	SetPause(true);
}

void AVSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (auto* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AVSPlayerController::Move);
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
		PC->OnPlayerDied.AddUObject(this, &AVSPlayerController::HandlePlayerDied);

	SetupHUD();
}

void AVSPlayerController::SetupHUD()
{
	AVSPlayerCharacter* PC = Cast<AVSPlayerCharacter>(GetPawn());
	if (!PC || !HUDWidgetClass) return;

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