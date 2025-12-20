// 物品槽位 Widget 实现

#include "ItemSlotWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Animation/WidgetAnimation.h"

void UItemSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 初始化为未选中状态
	bIsSelected = false;
	UpdateVisuals();
}

void UItemSlotWidget::SetItemData(const FItemSlot& ItemData)
{
	// 一次性更新所有显示
	SetItemIcon(ItemData.Icon);
	SetItemCount(ItemData.CurrentCount, ItemData.MaxCount);
	SetItemName(ItemData.DisplayName);
}

void UItemSlotWidget::SetItemIcon(UTexture2D* IconTexture)
{
	if (ItemIcon)
	{
		if (IconTexture)
		{
			ItemIcon->SetBrushFromTexture(IconTexture);
			ItemIcon->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			// 没有图标时显示空白或隐藏
			ItemIcon->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UItemSlotWidget::SetItemCount(int32 CurrentCount, int32 MaxCount)
{
	if (ItemCount)
	{
		// 格式化为 "当前/最大" 格式
		FString CountText = FString::Printf(TEXT("%d/%d"), CurrentCount, MaxCount);
		ItemCount->SetText(FText::FromString(CountText));

		// 根据是否有物品调整颜色
		if (CurrentCount > 0)
		{
			ItemCount->SetColorAndOpacity(FSlateColor(FLinearColor::White));
			if (ItemIcon)
			{
				ItemIcon->SetColorAndOpacity(AvailableItemColor);
			}
		}
		else
		{
			ItemCount->SetColorAndOpacity(FSlateColor(EmptyItemColor));
			if (ItemIcon)
			{
				ItemIcon->SetColorAndOpacity(EmptyItemColor);
			}
		}
	}
}

void UItemSlotWidget::SetItemName(const FText& Name)
{
	if (ItemName)
	{
		ItemName->SetText(Name);
	}
}

void UItemSlotWidget::SetSelected(bool bSelected)
{
	bIsSelected = bSelected;
	UpdateVisuals();
}

void UItemSlotWidget::PlayUseAnimation()
{
	// 播放蓝图中定义的使用动画
	if (UseAnimation)
	{
		PlayAnimation(UseAnimation);
	}
	else
	{
		// 如果没有动画，使用简单的闪烁效果
		if (Background)
		{
			// 闪烁效果：先变亮再恢复
			Background->SetRenderOpacity(0.5f);

			// 使用定时器恢复
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
			{
				if (Background)
				{
					Background->SetRenderOpacity(1.0f);
				}
			}, 0.1f, false);
		}
	}
}

void UItemSlotWidget::UpdateVisuals()
{
	// 更新选中边框
	if (SelectionBorder)
	{
		if (bIsSelected)
		{
			SelectionBorder->SetBrushColor(SelectedBorderColor);
			SelectionBorder->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			SelectionBorder->SetBrushColor(NormalBorderColor);
			// 未选中时可以选择隐藏边框或显示为暗色
			SelectionBorder->SetVisibility(ESlateVisibility::Visible);
		}
	}
}
