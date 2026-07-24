#include "UI/VSHUDWidget.h"
#include "ViewModel/VSHUDViewModel.h"

void UVSHUDWidget::SetViewModel(UVSHUDViewModel* InViewModel)
{
    ViewModel = InViewModel;
    OnViewModelSet();   // BP에서 MVVM 바인딩에 뷰모델 공급
}
