#include "VSUpgradeSelectionWidget.h"
#include "VSUpgradeCardWidget.h"
#include "Components/HorizontalBox.h"

void UVSUpgradeSelectionWidget::SetupChoices(const TArray<UVSUpgradeData*>& Choices)
{
    if (!CardContainer || !CardWidgetClass) return;

    CardContainer->ClearChildren();

    for (UVSUpgradeData* Upgrade : Choices)
    {
        UVSUpgradeCardWidget* Card = CreateWidget<UVSUpgradeCardWidget>(this, CardWidgetClass);
        if (Card)
        {
            Card->SetupCard(Upgrade);
            Card->OnSelected.AddDynamic(this, &UVSUpgradeSelectionWidget::HandleCardSelected);
            CardContainer->AddChild(Card);
        }
    }
}

void UVSUpgradeSelectionWidget::HandleCardSelected(UVSUpgradeData* Selected)
{
    OnUpgradeChosen.Broadcast(Selected);
}