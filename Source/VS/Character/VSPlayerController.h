#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "VSPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UVSPauseMenuWidget;

UCLASS()
class AVSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AVSPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> PauseAction;

	// 결과 화면 표시 (게임오버·클리어 공용). bIsVictory로 승/패 구분
	void ShowResult(bool bIsVictory);

	UFUNCTION(BlueprintCallable, Category = "VS|UI")
    void TogglePauseMenu();

    void OpenPauseMenu();
    void ClosePauseMenu();

    UFUNCTION(BlueprintCallable, Category = "VS|Flow")
    void RestartGame();

    UFUNCTION(BlueprintCallable, Category = "VS|Flow")
    void QuitGame();

protected:
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// Shipping 외에는 치트매니저를 항상 생성한다.
	virtual void AddCheats(bool bForce) override;
	
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);

protected:
	// 게임오버/클리어 공용 결과 위젯 클래스
	UPROPERTY(EditAnywhere, Category="VS|UI")
	TSoftClassPtr<class UVSResultWidget> ResultWidgetClass;

	// 인게임 HUD 위젯 클래스
	UPROPERTY(EditAnywhere, Category="VS|UI")
	TSoftClassPtr<class UVSHUDWidget> HUDWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "VS|UI")
    TSubclassOf<UVSPauseMenuWidget> PauseMenuWidgetClass;

private:
	void HandlePlayerDied();

	// 뷰모델 생성 → Model 연결 → HUD 위젯에 주입
	void SetupHUD();
	void TeardownHUD();

	void HandleRunCleared();

	void OnPauseGame(bool bPause);

private:
	UPROPERTY()
	TObjectPtr<class UVSHUDWidget> HUDWidget;

	UPROPERTY()
	TObjectPtr<class UVSHUDViewModel> HUDViewModel;

	FDelegateHandle PlayerDiedHandle;

	UPROPERTY(Transient)
    TObjectPtr<UVSPauseMenuWidget> PauseMenuWidget;

	bool bGameOver = false;

	// 일시정지 메뉴를 열기 직전의 커서 표시 상태
	bool bCursorVisibleBeforePause = false;
};