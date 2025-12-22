// 商店管理器实现

#include "ShopManager.h"
#include "../WukongCharacter.h"
#include "../Components/WalletComponent.h"
#include "../Components/InventoryComponent.h"

FShopItemData UShopManager::EmptyShopItem;

UShopManager::UShopManager()
{
	// 构造时初始化默认商品
	InitializeDefaultItems();
}

void UShopManager::InitializeDefaultItems()
{
	ShopItems.Empty();
	PurchaseCounts.Empty();

	// 血药
	FShopItemData HealthPotion;
	HealthPotion.ItemType = EItemType::HealthPotion;
	HealthPotion.DisplayName = FText::FromString(TEXT("金创药"));
	HealthPotion.Description = FText::FromString(TEXT("恢复50点生命值"));
	HealthPotion.Price = 100;
	HealthPotion.PurchaseLimit = 0;  // 无限购买
	ShopItems.Add(HealthPotion);

	// 体力药
	FShopItemData StaminaPotion;
	StaminaPotion.ItemType = EItemType::StaminaPotion;
	StaminaPotion.DisplayName = FText::FromString(TEXT("灵气丹"));
	StaminaPotion.Description = FText::FromString(TEXT("恢复50点体力"));
	StaminaPotion.Price = 80;
	StaminaPotion.PurchaseLimit = 0;  // 无限购买
	ShopItems.Add(StaminaPotion);

	// 攻击Buff
	FShopItemData AttackBuff;
	AttackBuff.ItemType = EItemType::AttackBuff;
	AttackBuff.DisplayName = FText::FromString(TEXT("怒火丹"));
	AttackBuff.Description = FText::FromString(TEXT("攻击力提升30%，持续10秒"));
	AttackBuff.Price = 200;
	AttackBuff.PurchaseLimit = 5;  // 限购5个
	ShopItems.Add(AttackBuff);

	// 防御Buff
	FShopItemData DefenseBuff;
	DefenseBuff.ItemType = EItemType::DefenseBuff;
	DefenseBuff.DisplayName = FText::FromString(TEXT("金刚丹"));
	DefenseBuff.Description = FText::FromString(TEXT("受伤减免50%，持续10秒"));
	DefenseBuff.Price = 200;
	DefenseBuff.PurchaseLimit = 5;  // 限购5个
	ShopItems.Add(DefenseBuff);
}

const FShopItemData& UShopManager::GetShopItem(int32 Index) const
{
	if (Index >= 0 && Index < ShopItems.Num())
	{
		return ShopItems[Index];
	}
	return EmptyShopItem;
}

bool UShopManager::CanPurchase(AWukongCharacter* Customer, int32 ItemIndex)
{
	if (!Customer || ItemIndex < 0 || ItemIndex >= ShopItems.Num())
	{
		return false;
	}

	const FShopItemData& Item = ShopItems[ItemIndex];

	// 检查金币是否足够
	UWalletComponent* Wallet = Customer->GetWalletComponent();
	if (!Wallet || !Wallet->HasEnoughGold(Item.Price))
	{
		return false;
	}

	// 检查是否达到购买上限
	if (Item.PurchaseLimit > 0)
	{
		int32 CurrentPurchases = PurchaseCounts.FindRef(ItemIndex);
		if (CurrentPurchases >= Item.PurchaseLimit)
		{
			return false;
		}
	}

	// 检查背包是否已满
	UInventoryComponent* Inventory = Customer->GetInventoryComponent();
	if (Inventory)
	{
		// 查找对应物品槽位，检查是否已满
		for (int32 i = 0; i < Inventory->GetSlotCount(); i++)
		{
			const FItemSlot& Slot = Inventory->GetItemSlot(i);
			if (Slot.ItemType == Item.ItemType)
			{
				if (Slot.CurrentCount >= Slot.MaxCount)
				{
					return false;  // 该类型物品已满
				}
				break;
			}
		}
	}

	return true;
}

bool UShopManager::PurchaseItem(AWukongCharacter* Customer, int32 ItemIndex)
{
	if (!CanPurchase(Customer, ItemIndex))
	{
		return false;
	}

	const FShopItemData& Item = ShopItems[ItemIndex];

	// 扣除金币
	UWalletComponent* Wallet = Customer->GetWalletComponent();
	if (!Wallet->SpendGold(Item.Price))
	{
		return false;
	}

	// 添加物品到背包
	UInventoryComponent* Inventory = Customer->GetInventoryComponent();
	if (Inventory)
	{
		Inventory->AddItemByType(Item.ItemType, 1);
	}

	// 记录购买次数
	if (Item.PurchaseLimit > 0)
	{
		int32& Count = PurchaseCounts.FindOrAdd(ItemIndex);
		Count++;
	}

	return true;
}

int32 UShopManager::GetRemainingPurchases(int32 ItemIndex) const
{
	if (ItemIndex < 0 || ItemIndex >= ShopItems.Num())
	{
		return -1;
	}

	const FShopItemData& Item = ShopItems[ItemIndex];

	// 无限购买
	if (Item.PurchaseLimit == 0)
	{
		return 0;
	}

	int32 CurrentPurchases = PurchaseCounts.FindRef(ItemIndex);
	int32 Remaining = Item.PurchaseLimit - CurrentPurchases;

	return Remaining > 0 ? Remaining : -1;
}

void UShopManager::ResetPurchaseCounts()
{
	PurchaseCounts.Empty();
}
