#include "AlertIconWidget.h"
#include "Components/Image.h"
#include "Animation/WidgetAnimation.h"

// Widget 初始化
void UAlertIconWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 初始化时设置默认颜色
	if (AlertIcon)
	{
		AlertIcon->SetColorAndOpacity(DefaultIconColor);
	}

	bIsAnimating = false;
}

// 设置图标可见性
void UAlertIconWidget::SetIconVisibility(bool bVisible)
{
	if (AlertIcon)
	{
		AlertIcon->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

		// 如果显示，设置为警戒颜色
		if (bVisible)
		{
			AlertIcon->SetColorAndOpacity(AlertedIconColor);
		}
		else
		{
			// 隐藏时停止动画
			StopAlertAnimation();
			AlertIcon->SetColorAndOpacity(DefaultIconColor);
		}
	}
}

// 播放警戒动画
void UAlertIconWidget::PlayAlertAnimation()
{
	if (AlertAnimation && !bIsAnimating)
	{
		PlayAnimation(AlertAnimation, 0.0f, 0); // 0 = 循环播放
		bIsAnimating = true;
	}
}

// 停止警戒动画
void UAlertIconWidget::StopAlertAnimation()
{
	if (AlertAnimation && bIsAnimating)
	{
		StopAnimation(AlertAnimation);
		bIsAnimating = false;
	}
}
