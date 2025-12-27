// 商品条目Widget - 显示单个商品信息

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopItemWidget.generated.h"

class UImage;
class UTextBlock;
class UButton;
class UBorder;
class UTradeMenuWidget;
struct FShopItemData;

UCLASS()
class BLACKMYTH_API UShopItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// === UI控件绑定 ===

	// 商品图标
	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	// 商品名称
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemNameText;

	// 商品价格
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemPriceText;

	// 拥有数量
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemCountText;

	// 选择按钮
	UPROPERTY(meta = (BindWidget))
	UButton* SelectButton;

	// 选中状态边框（可选，用于高亮显示）
	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* SelectionBorder;

	// === 逻辑属性 ===

	// 商品索引
	int32 ItemIndex = -1;

	// 父级商店Widget引用
	UPROPERTY()
	UTradeMenuWidget* ParentShopWidget;

	// 是否被选中
	bool bIsSelected = false;

	// === 生命周期 ===

	virtual void NativeConstruct() override;

	// === 方法 ===

	// 设置商品数据
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void SetItemData(const FShopItemData& ItemData, int32 Index, int32 OwnedCount);

	// 设置选中状态
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void SetSelected(bool bSelected);

	// 点击回调
	UFUNCTION()
	void OnSelectClicked();

private:
	// 绑定按钮事件
	void BindSelectButton();

	// 更新选中状态的视觉效果
	void UpdateSelectionVisual();
};
