#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ViewModel/VSBossHeadBarViewModel.h"
#include "VSBossHeadBarWidget.generated.h"

UCLASS()
class VS_API UVSBossHeadBarWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Boss")
    void SetViewModel(UVSBossHeadBarViewModel* InViewModel);

    UFUNCTION(BlueprintPure, Category="Boss")
    UVSBossHeadBarViewModel* GetViewModel() const { return ViewModel; }

protected:
    UFUNCTION(BlueprintImplementableEvent, Category="Boss")
    void OnViewModelSet();

private:
    UPROPERTY(Transient)
    TObjectPtr<UVSBossHeadBarViewModel> ViewModel;
};