// 商店管理器 - 处理商品购买逻辑

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ShopTypes.h"
#include "ShopManager.generated.h"

class AWukongCharacter;

UCLASS(BlueprintType)
class BLACKMYTH_API UShopManager : public UObject
{
	GENERATED_BODY()

public:
	UShopManager();

	// 商品列表（在蓝图中配置或硬编码默认值）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	TArray<FShopItemData> ShopItems;

	// 初始化默认商品
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void InitializeDefaultItems();

	// 获取商品列表
	const TArray<FShopItemData>& GetShopItems() const { return ShopItems; }

	// 获取商品数量
	UFUNCTION(BlueprintPure, Category = "Shop")
	int32 GetShopItemCount() const { return ShopItems.Num(); }

	// 获取指定索引的商品
	UFUNCTION(BlueprintPure, Category = "Shop")
	const FShopItemData& GetShopItem(int32 Index) const;

	// 检查是否可以购买
	UFUNCTION(BlueprintCallable, Category = "Shop")
	bool CanPurchase(AWukongCharacter* Customer, int32 ItemIndex);

	// 执行购买
	UFUNCTION(BlueprintCallable, Category = "Shop")
	bool PurchaseItem(AWukongCharacter* Customer, int32 ItemIndex);

	// 获取商品剩余可购买次数（0=无限，-1=已达上限）
	UFUNCTION(BlueprintPure, Category = "Shop")
	int32 GetRemainingPurchases(int32 ItemIndex) const;

	// 重置购买计数（新游戏或特定事件时调用）
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void ResetPurchaseCounts();

private:
	// 记录每个商品的购买次数（用于限购）
	TMap<int32, int32> PurchaseCounts;

	// 空商品数据（用于返回无效索引时）
	static FShopItemData EmptyShopItem;
};
