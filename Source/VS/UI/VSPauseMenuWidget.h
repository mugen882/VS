// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VSPauseMenuWidget.generated.h"

class UButton;
class UWidget;

/**
 * ESC 일시정지 메뉴. 계속하기 / 재시작 / 종료.
 * 종료 확인 팝업(QuitConfirmPanel)을 내부에 품고 Visibility로만 토글한다.
 */
UCLASS()
class VS_API UVSPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * ESC(뒤로가기) 입력 처리.
	 * @return 종료 확인 팝업이 열려 있어서 팝업만 닫았으면 true (메뉴는 유지).
	 *         닫을 팝업이 없으면 false — 호출자가 메뉴 자체를 닫으면 된다.
	 */
	bool HandleBackPressed();

	/**
	 * 메뉴에 키보드 포커스를 준다.
	 */
	void FocusMenu();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/**
	 * 메뉴가 떠 있는 동안의 ESC는 여기서 받는다.
	 */
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// --- 메뉴 ---

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnResume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnRestart;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnQuit;

	// --- 종료 확인 팝업 ---

	/** 팝업 전체를 감싸는 컨테이너. 이것만 켜고 끈다. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> QuitConfirmPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnQuitYes;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> BtnQuitNo;

private:
	UFUNCTION()
	void HandleResumeClicked();

	UFUNCTION()
	void HandleRestartClicked();

	UFUNCTION()
	void HandleQuitClicked();

	UFUNCTION()
	void HandleQuitYesClicked();

	UFUNCTION()
	void HandleQuitNoClicked();

	void OpenQuitConfirm();
	void CloseQuitConfirm();
	bool IsQuitConfirmOpen() const;

	/** 재시작/종료 실행 중 중복 클릭 방지 */
	bool bClosing = false;
};
