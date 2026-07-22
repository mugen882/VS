#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VSUpgradeCardWidget.generated.h"

class UVSUpgradeData;
class UButton;
class UTextBlock;
class UImage;

// 카드 선택 시 알림용 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeCardSelected, UVSUpgradeData*, SelectedUpgrade);

UCLASS()
class VS_API UVSUpgradeCardWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    // 이 카드가 표시할 업그레이드 데이터 세팅
    void SetupCard(UVSUpgradeData* InUpgrade);

    // 카드 선택 이벤트 (선택 화면이 구독)
    FOnUpgradeCardSelected OnSelected;

protected:
    virtual void NativeConstruct() override;

    // BP 디자이너에서 만든 위젯과 바인딩 (이름 일치 필수)
    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UButton> SelectButton;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UTextBlock> TitleText;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UTextBlock> DescText;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UImage> IconImage;

    UPROPERTY(meta=(BindWidget))
    TObjectPtr<UTextBlock> ButtonText;

private:
    UPROPERTY()
    TObjectPtr<UVSUpgradeData> Upgrade;

    UFUNCTION()
    void HandleButtonClicked();
};