#include "UI/VSBossHeadBarWidget.h"

void UVSBossHeadBarWidget::SetViewModel(UVSBossHeadBarViewModel* InViewModel)
{
    ViewModel = InViewModel;
    OnViewModelSet();   // BP에서 MVVM 바인딩에 뷰모델 공급
}
