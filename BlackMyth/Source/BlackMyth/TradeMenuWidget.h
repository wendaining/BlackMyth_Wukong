// 商店交易菜单Widget - 展示商品列表并处理购买

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TradeMenuWidget.generated.h"

class UScrollBox;
class UTextBlock;
class UButton;
class UShopManager;
class AWukongCharacter;
class UShopItemWidget;

UCLASS()
class BLACKMYTH_API UTradeMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 返回TempleMenu引用
	UPROPERTY()
	UUserWidget* OwnerTempleWidget;

	// === UI控件绑定 ===

	// 商品列表容器
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ItemListBox;

	// 金币显示
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GoldText;

	// 选中商品详情
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SelectedItemName;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SelectedItemDesc;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SelectedItemPrice;

	// 购买按钮
	UPROPERTY(meta = (BindWidget))
	UButton* PurchaseButton;

	// === 配置 ===

	// 商品条目Widget类（在蓝图中配置）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	TSubclassOf<UShopItemWidget> ShopItemWidgetClass;

	// === 逻辑属性 ===

	// 商店管理器
	UPROPERTY()
	UShopManager* ShopManager;

	// 玩家引用
	UPROPERTY()
	AWukongCharacter* CustomerPlayer;

	// 当前选中的商品索引
	int32 SelectedItemIndex = -1;

	// 已生成的商品条目Widget列表
	UPROPERTY()
	TArray<UShopItemWidget*> ItemWidgets;

	// === 生命周期 ===

	virtual void NativeConstruct() override;

	// === 初始化与刷新 ===

	// 初始化商店
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void InitializeShop();

	// 刷新金币显示（无参版本供外部调用）
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void RefreshGoldDisplay();

	// 刷新金币显示（带参数版本供委托回调使用）
	UFUNCTION()
	void OnGoldChanged(int32 NewGold);

	// 刷新商品列表
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void RefreshItemList();

	// 刷新选中商品的详情
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void RefreshSelectedItemDetails();

	// === 交互回调 ===

	// 商品被选中
	UFUNCTION()
	void OnItemSelected(int32 ItemIndex);

	// 购买按钮点击
	UFUNCTION()
	void OnPurchaseClicked();

	// 返回按钮（已有）
	UFUNCTION(BlueprintCallable)
	void OnBackClicked();

private:
	// 绑定购买按钮事件
	void BindPurchaseButton();

	// 更新购买按钮状态（可用/禁用）
	void UpdatePurchaseButtonState();

	// 清空商品列表
	void ClearItemList();
};
