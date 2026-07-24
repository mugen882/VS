#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VSHUDWidget.generated.h"

class UVSHUDViewModel;

/**
 * 인게임 HUD.
 * 컨트롤러가 만든 뷰모델을 받아 BP의 MVVM 바인딩에 공급한다.
*/
UCLASS()
class VS_API UVSHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="HUD")
    void SetViewModel(UVSHUDViewModel* InViewModel);

    UFUNCTION(BlueprintPure, Category="HUD")
    UVSHUDViewModel* GetViewModel() const { return ViewModel; }

protected:
    UFUNCTION(BlueprintImplementableEvent, Category="HUD")
    void OnViewModelSet();

private:
    UPROPERTY()
    TObjectPtr<UVSHUDViewModel> ViewModel;
};
