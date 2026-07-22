#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VSUpgradeSelectionWidget.generated.h"

class UVSUpgradeData;
class UVSUpgradeCardWidget;
class UHorizontalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeChosen, UVSUpgradeData*, ChosenUpgrade);

UCLASS()
class VS_API UVSUpgradeSelectionWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    void SetupChoices(const TArray<UVSUpgradeData*>& Choices);

    FOnUpgradeChosen OnUpgradeChosen;

protected:
    // 카드들을 담을 컨테이너
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UHorizontalBox> CardContainer;

    // 카드 위젯 클래스
    UPROPERTY(EditAnywhere, Category="Upgrade")
    TSubclassOf<UVSUpgradeCardWidget> CardWidgetClass;

private:
    UFUNCTION()
    void HandleCardSelected(UVSUpgradeData* Selected);
};