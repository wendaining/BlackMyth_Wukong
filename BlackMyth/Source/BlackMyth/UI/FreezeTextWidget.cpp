// 定身术"定"字 UI Widget 实现

#include "FreezeTextWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UFreezeTextWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 设置默认的"定"字文本
	if (FreezeText)
	{
		FreezeText->SetText(FText::FromString(TEXT("定")));
	}

	// 播放出现动画（如果蓝图实现了）
	PlayAppearAnimation();
}

void UFreezeTextWidget::SetTextColor(FLinearColor Color)
{
	if (FreezeText)
	{
		FreezeText->SetColorAndOpacity(FSlateColor(Color));
	}
}
