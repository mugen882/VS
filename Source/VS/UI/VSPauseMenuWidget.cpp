// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/VSPauseMenuWidget.h"

#include "Components/Button.h"
#include "Character/VSPlayerController.h"
#include "Framework/Application/SlateApplication.h"

void UVSPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (BtnResume)
	{
		BtnResume->OnClicked.AddUniqueDynamic(this, &UVSPauseMenuWidget::HandleResumeClicked);
	}

	if (BtnRestart)
	{
		BtnRestart->OnClicked.AddUniqueDynamic(this, &UVSPauseMenuWidget::HandleRestartClicked);
	}

	if (BtnQuit)
	{
		BtnQuit->OnClicked.AddUniqueDynamic(this, &UVSPauseMenuWidget::HandleQuitClicked);
	}

	if (BtnQuitYes)
	{
		BtnQuitYes->OnClicked.AddUniqueDynamic(this, &UVSPauseMenuWidget::HandleQuitYesClicked);
	}

	if (BtnQuitNo)
	{
		BtnQuitNo->OnClicked.AddUniqueDynamic(this, &UVSPauseMenuWidget::HandleQuitNoClicked);
	}

	// 항상 닫힌 상태로 시작
	CloseQuitConfirm();

	// ESC 키를 받으려면 포커스가 필요하다
	SetIsFocusable(true);
	FocusMenu();
}

void UVSPauseMenuWidget::NativeDestruct()
{
	if (BtnResume)
	{
		BtnResume->OnClicked.RemoveDynamic(this, &UVSPauseMenuWidget::HandleResumeClicked);
	}

	if (BtnRestart)
	{
		BtnRestart->OnClicked.RemoveDynamic(this, &UVSPauseMenuWidget::HandleRestartClicked);
	}

	if (BtnQuit)
	{
		BtnQuit->OnClicked.RemoveDynamic(this, &UVSPauseMenuWidget::HandleQuitClicked);
	}

	if (BtnQuitYes)
	{
		BtnQuitYes->OnClicked.RemoveDynamic(this, &UVSPauseMenuWidget::HandleQuitYesClicked);
	}

	if (BtnQuitNo)
	{
		BtnQuitNo->OnClicked.RemoveDynamic(this, &UVSPauseMenuWidget::HandleQuitNoClicked);
	}

	Super::NativeDestruct();
}

void UVSPauseMenuWidget::FocusMenu()
{
	if (BtnResume)
	{
		BtnResume->SetKeyboardFocus();
		return;
	}

	SetKeyboardFocus();
}

FReply UVSPauseMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape && !bClosing)
	{
		// 팝업이 열려 있으면 팝업만, 아니면 메뉴 전체를 닫는다
		if (!HandleBackPressed())
		{
			if (AVSPlayerController* PC = GetOwningPlayer<AVSPlayerController>())
			{
				PC->ClosePauseMenu();
			}
		}

		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

bool UVSPauseMenuWidget::HandleBackPressed()
{
	if (IsQuitConfirmOpen())
	{
		CloseQuitConfirm();
		return true;
	}

	return false;
}

void UVSPauseMenuWidget::HandleResumeClicked()
{
	if (bClosing)
	{
		return;
	}

	if (AVSPlayerController* PC = GetOwningPlayer<AVSPlayerController>())
	{
		PC->ClosePauseMenu();
	}
}

void UVSPauseMenuWidget::HandleRestartClicked()
{
	if (bClosing)
	{
		return;
	}

	bClosing = true;

	if (AVSPlayerController* PC = GetOwningPlayer<AVSPlayerController>())
	{
		PC->RestartGame();
	}
}

void UVSPauseMenuWidget::HandleQuitClicked()
{
	if (bClosing)
	{
		return;
	}

	OpenQuitConfirm();
}

void UVSPauseMenuWidget::HandleQuitYesClicked()
{
	if (bClosing)
	{
		return;
	}

	bClosing = true;

	if (AVSPlayerController* PC = GetOwningPlayer<AVSPlayerController>())
	{
		PC->QuitGame();
	}
}

void UVSPauseMenuWidget::HandleQuitNoClicked()
{
	CloseQuitConfirm();
	FocusMenu();
}

void UVSPauseMenuWidget::OpenQuitConfirm()
{
	if (!QuitConfirmPanel)
	{
		return;
	}

	QuitConfirmPanel->SetVisibility(ESlateVisibility::Visible);

	if (BtnQuitNo)
	{
		// 기본 포커스를 "아니오"에 — 엔터 연타로 실수 종료하는 걸 방지
		BtnQuitNo->SetKeyboardFocus();
	}
}

void UVSPauseMenuWidget::CloseQuitConfirm()
{
	if (QuitConfirmPanel)
	{
		QuitConfirmPanel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

bool UVSPauseMenuWidget::IsQuitConfirmOpen() const
{
	return QuitConfirmPanel && QuitConfirmPanel->GetVisibility() == ESlateVisibility::Visible;
}
