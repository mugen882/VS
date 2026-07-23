#include "VSUpgradeCardWidget.h"
#include "Data/VSUpgradeData.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Common/VSStringData.h"

void UVSUpgradeCardWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (SelectButton)
        SelectButton->OnClicked.AddDynamic(this, &UVSUpgradeCardWidget::HandleButtonClicked);
}

void UVSUpgradeCardWidget::SetupCard(UVSUpgradeData* InUpgrade)
{
    Upgrade = InUpgrade;
    if (!Upgrade) return;

    if (TitleText) TitleText->SetText(Upgrade->Title);
    if (DescText)  DescText->SetText(Upgrade->Description);
    if (IconImage && Upgrade->Icon)
        IconImage->SetBrushFromTexture(Upgrade->Icon);
    
    switch (Upgrade->Type)
    {
        case EVSUpgradeType::NewWeapon:
            ButtonText->SetText(FText::FromString(strGain));
			break;
        case EVSUpgradeType::UpgradeWeapon:
            ButtonText->SetText(FText::FromString(strUpgrade));
            break;
		case EVSUpgradeType::Passive:
            ButtonText->SetText(FText::FromString(strUpgrade));
			break;

        default:
        break;
    }
}

void UVSUpgradeCardWidget::HandleButtonClicked()
{
    OnSelected.Broadcast(Upgrade);
}