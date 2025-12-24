// 金币价值显示Widget

#include "GoldValueWidget.h"
#include "Components/TextBlock.h"

void UGoldValueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 默认样式（可在蓝图中覆盖）
	if (ValueText)
	{
		ValueText->SetColorAndOpacity(FLinearColor(1.0f, 0.84f, 0.0f, 1.0f));  // 金色
		ValueText->SetJustification(ETextJustify::Center);
	}
}

void UGoldValueWidget::SetGoldValue(int32 Value)
{
	if (ValueText)
	{
		ValueText->SetText(FText::AsNumber(Value));
		UE_LOG(LogTemp, Log, TEXT("[GoldValueWidget] Set value to %d"), Value);
	}
}
