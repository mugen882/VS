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
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	// 게임오버/클리어 공용 결과 위젯 클래스
	UPROPERTY(EditAnywhere, Category="UI")
	TSoftClassPtr<class UVSResultWidget> ResultWidgetClass;

	// 인게임 HUD 위젯 클래스
	UPROPERTY(EditAnywhere, Category="UI")
	TSoftClassPtr<class UVSHUDWidget> HUDWidgetClass;

	// 결과 화면 표시 (게임오버·클리어 공용). bIsVictory로 승/패 구분
	void ShowResult(bool bIsVictory);

protected:
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	// Shipping 외에는 치트매니저를 항상 생성한다.
	virtual void AddCheats(bool bForce) override;
	
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);

private:
	void HandlePlayerDied();

	// 뷰모델 생성 → Model 연결 → HUD 위젯에 주입
	void SetupHUD();
	void TeardownHUD();

	void HandleRunCleared();

private:
	UPROPERTY()
	TObjectPtr<class UVSHUDWidget> HUDWidget;

	UPROPERTY()
	TObjectPtr<class UVSHUDViewModel> HUDViewModel;

	FDelegateHandle PlayerDiedHandle;
};