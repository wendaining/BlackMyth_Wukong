// 商品条目Widget实现

#include "ShopItemWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "../TradeMenuWidget.h"
#include "../Shop/ShopTypes.h"

void UShopItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindSelectButton();
	UpdateSelectionVisual();
}

void UShopItemWidget::SetItemData(const FShopItemData& ItemData, int32 Index, int32 OwnedCount)
{
	ItemIndex = Index;

	// 设置商品图标
	if (ItemIcon && ItemData.Icon)
	{
		ItemIcon->SetBrushFromTexture(ItemData.Icon);
		ItemIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else if (ItemIcon)
	{
		ItemIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	// 设置商品名称
	if (ItemNameText)
	{
		ItemNameText->SetText(ItemData.DisplayName);
	}

	// 设置商品价格
	if (ItemPriceText)
	{
		FString PriceString = FString::Printf(TEXT("%d"), ItemData.Price);
		ItemPriceText->SetText(FText::FromString(PriceString));
	}

	// 设置拥有数量
	if (ItemCountText)
	{
		FString CountString = FString::Printf(TEXT("拥有: %d"), OwnedCount);
		ItemCountText->SetText(FText::FromString(CountString));
	}
}

void UShopItemWidget::SetSelected(bool bSelected)
{
	bIsSelected = bSelected;
	UpdateSelectionVisual();
}

void UShopItemWidget::OnSelectClicked()
{
	if (ParentShopWidget && ItemIndex >= 0)
	{
		ParentShopWidget->OnItemSelected(ItemIndex);
	}
}

void UShopItemWidget::BindSelectButton()
{
	if (SelectButton)
	{
		SelectButton->OnClicked.AddDynamic(this, &UShopItemWidget::OnSelectClicked);
	}
}

void UShopItemWidget::UpdateSelectionVisual()
{
	// 如果有选中边框，根据选中状态更新显示
	if (SelectionBorder)
	{
		if (bIsSelected)
		{
			// 选中时显示高亮边框（金色）
			SelectionBorder->SetBrushColor(FLinearColor(1.0f, 0.8f, 0.0f, 1.0f));
		}
		else
		{
			// 未选中时透明
			SelectionBorder->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
		}
	}
}
