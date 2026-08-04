#pragma once

#include "CoreMinimal.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "VSPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class AVSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AVSPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	// 게임오버/클리어 공용 결과 위젯 클래스
	UPROPERTY(EditAnywhere, Category="UI")
	TSoftClassPtr<class UVSResultWidget> ResultWidgetClass;

	// 인게임 HUD 위젯 클래스
	UPROPERTY(EditAnywhere, Category="UI")
	TSoftClassPtr<class UVSHUDWidget> HUDWidgetClass;

	// 결과 화면 표시 (게임오버·클리어 공용). bIsVictory로 승/패 구분
	void ShowResult(bool bIsVictory);

	// --- 성능 측정 콘솔 명령 ---
	// exec 함수는 PlayerController 계열에서만 콘솔로 라우팅되므로 여기에 두고
	// 실제 작업은 레벨에 배치된 AVSBenchmarkActor에 위임한다.
	UFUNCTION(Exec)
	void VSBench(int32 Count);        // ISM + AnimToTexture 경로

	UFUNCTION(Exec)
	void VSBenchActor(int32 Count);   // 액터 + 스켈레탈 경로

	UFUNCTION(Exec)
	void VSBenchClear();
	
protected:
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	
	virtual void BeginPlay();

	void Move(const FInputActionValue& Value);

private:
	void HandlePlayerDied();

	// 뷰모델 생성 → Model 연결 → HUD 위젯에 주입
	void SetupHUD();

private:
	UPROPERTY()
	TObjectPtr<class UVSHUDWidget> HUDWidget;

	UPROPERTY()
	TObjectPtr<class UVSHUDViewModel> HUDViewModel;
};