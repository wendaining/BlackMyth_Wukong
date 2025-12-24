// 商店交易菜单Widget实现

#include "TradeMenuWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Shop/ShopManager.h"
#include "Shop/ShopTypes.h"
#include "UI/ShopItemWidget.h"
#include "WukongCharacter.h"
#include "Components/WalletComponent.h"
#include "Components/InventoryComponent.h"
#include "Kismet/GameplayStatics.h"

void UTradeMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindPurchaseButton();
	InitializeShop();
}

void UTradeMenuWidget::InitializeShop()
{
	// 获取玩家引用
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	CustomerPlayer = Cast<AWukongCharacter>(PlayerPawn);

	if (!CustomerPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("TradeMenuWidget: 无法获取玩家引用"));
		return;
	}

	// 创建商店管理器
	if (!ShopManager)
	{
		ShopManager = NewObject<UShopManager>(this);
		ShopManager->InitializeFromInventory(CustomerPlayer);
	}

	// 绑定金币变化事件
	if (UWalletComponent* Wallet = CustomerPlayer->GetWalletComponent())
	{
		Wallet->OnGoldChanged.AddDynamic(this, &UTradeMenuWidget::OnGoldChanged);
	}

	// 刷新UI
	RefreshGoldDisplay();
	RefreshItemList();
}

void UTradeMenuWidget::RefreshGoldDisplay()
{
	if (!GoldText)
	{
		UE_LOG(LogTemp, Warning, TEXT("TradeMenuWidget: GoldText 控件为空，请检查蓝图中控件名称是否为 GoldText"));
		return;
	}

	if (!CustomerPlayer)
	{
		UE_LOG(LogTemp, Warning, TEXT("TradeMenuWidget: CustomerPlayer 为空"));
		return;
	}

	UWalletComponent* Wallet = CustomerPlayer->GetWalletComponent();
	if (!Wallet)
	{
		UE_LOG(LogTemp, Warning, TEXT("TradeMenuWidget: WalletComponent 为空，请确认玩家蓝图中添加了 WalletComponent"));
		return;
	}

	FString GoldString = FString::Printf(TEXT("金币: %d"), Wallet->GetGold());
	GoldText->SetText(FText::FromString(GoldString));
	UE_LOG(LogTemp, Log, TEXT("TradeMenuWidget: 金币显示已更新为 %d"), Wallet->GetGold());
}

void UTradeMenuWidget::OnGoldChanged(int32 NewGold)
{
	// 委托回调，直接刷新UI
	RefreshGoldDisplay();
}

void UTradeMenuWidget::RefreshItemList()
{
	if (!ItemListBox || !ShopManager)
	{
		return;
	}

	// 清空现有列表
	ClearItemList();

	// 检查Widget类是否配置
	if (!ShopItemWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("TradeMenuWidget: ShopItemWidgetClass 未配置"));
		return;
	}

	// 为每个商品创建Widget
	const TArray<FShopItemData>& Items = ShopManager->GetShopItems();
	for (int32 i = 0; i < Items.Num(); i++)
	{
		UShopItemWidget* ItemWidget = CreateWidget<UShopItemWidget>(this, ShopItemWidgetClass);
		if (ItemWidget)
		{
			// 获取玩家已拥有的该物品数量
			int32 OwnedCount = 0;
			if (CustomerPlayer)
			{
				if (UInventoryComponent* Inventory = CustomerPlayer->GetInventoryComponent())
				{
					OwnedCount = Inventory->GetItemCount(Items[i].ItemType);
				}
			}

			// 设置商品数据
			ItemWidget->SetItemData(Items[i], i, OwnedCount);
			ItemWidget->ParentShopWidget = this;

			// 添加到ScrollBox
			ItemListBox->AddChild(ItemWidget);
			ItemWidgets.Add(ItemWidget);
		}
	}

	// 默认选中第一个商品
	if (Items.Num() > 0)
	{
		OnItemSelected(0);
	}
}

void UTradeMenuWidget::RefreshSelectedItemDetails()
{
	if (!ShopManager || SelectedItemIndex < 0)
	{
		// 无选中商品时清空详情
		if (SelectedItemName)
		{
			SelectedItemName->SetText(FText::GetEmpty());
		}
		if (SelectedItemDesc)
		{
			SelectedItemDesc->SetText(FText::GetEmpty());
		}
		if (SelectedItemPrice)
		{
			SelectedItemPrice->SetText(FText::GetEmpty());
		}
		return;
	}

	const FShopItemData& Item = ShopManager->GetShopItem(SelectedItemIndex);

	if (SelectedItemName)
	{
		SelectedItemName->SetText(Item.DisplayName);
	}

	if (SelectedItemDesc)
	{
		SelectedItemDesc->SetText(Item.Description);
	}

	if (SelectedItemPrice)
	{
		// 显示价格和购买限制信息
		FString PriceString;

		if (Item.PurchaseLimit == 0)
		{
			// 无限购买 - 显示"不限购"
			PriceString = FString::Printf(TEXT("价格: %d 金币 (不限购)"), Item.Price);
		}
		else
		{
			int32 RemainingPurchases = ShopManager->GetRemainingPurchases(SelectedItemIndex);
			if (RemainingPurchases == -1)
			{
				// 已达上限
				PriceString = FString::Printf(TEXT("价格: %d 金币 (已售罄)"), Item.Price);
			}
			else
			{
				// 显示剩余次数
				PriceString = FString::Printf(TEXT("价格: %d 金币 (剩余 %d)"), Item.Price, RemainingPurchases);
			}
		}

		SelectedItemPrice->SetText(FText::FromString(PriceString));
	}

	UpdatePurchaseButtonState();
}

void UTradeMenuWidget::OnItemSelected(int32 ItemIndex)
{
	SelectedItemIndex = ItemIndex;

	// 更新所有条目的选中状态视觉效果（由各ItemWidget自己处理）
	for (int32 i = 0; i < ItemWidgets.Num(); i++)
	{
		if (ItemWidgets[i])
		{
			ItemWidgets[i]->SetSelected(i == ItemIndex);
		}
	}

	RefreshSelectedItemDetails();
}

void UTradeMenuWidget::OnPurchaseClicked()
{
	if (!ShopManager || !CustomerPlayer || SelectedItemIndex < 0)
	{
		return;
	}

	// 尝试购买
	if (ShopManager->PurchaseItem(CustomerPlayer, SelectedItemIndex))
	{
		UE_LOG(LogTemp, Log, TEXT("购买成功: 商品索引 %d"), SelectedItemIndex);

		// 刷新UI
		RefreshGoldDisplay();
		RefreshItemList();  // 刷新列表以更新拥有数量
		RefreshSelectedItemDetails();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("购买失败: 商品索引 %d"), SelectedItemIndex);
	}
}

void UTradeMenuWidget::OnBackClicked()
{
	// 解绑金币变化事件
	if (CustomerPlayer)
	{
		if (UWalletComponent* Wallet = CustomerPlayer->GetWalletComponent())
		{
			Wallet->OnGoldChanged.RemoveDynamic(this, &UTradeMenuWidget::OnGoldChanged);
		}
	}

	RemoveFromParent();

	if (OwnerTempleWidget)
	{
		OwnerTempleWidget->AddToViewport();
	}
}

void UTradeMenuWidget::BindPurchaseButton()
{
	if (PurchaseButton)
	{
		PurchaseButton->OnClicked.AddDynamic(this, &UTradeMenuWidget::OnPurchaseClicked);
	}
}

void UTradeMenuWidget::UpdatePurchaseButtonState()
{
	if (!PurchaseButton || !ShopManager || !CustomerPlayer)
	{
		return;
	}

	// 检查是否可以购买当前选中的商品
	bool bCanPurchase = ShopManager->CanPurchase(CustomerPlayer, SelectedItemIndex);
	PurchaseButton->SetIsEnabled(bCanPurchase);
}

void UTradeMenuWidget::ClearItemList()
{
	if (ItemListBox)
	{
		ItemListBox->ClearChildren();
	}
	ItemWidgets.Empty();
	SelectedItemIndex = -1;
}
