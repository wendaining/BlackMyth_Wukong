// 商店系统 - 商品数据结构定义

#pragma once

#include "CoreMinimal.h"
#include "../Items/ItemTypes.h"
#include "ShopTypes.generated.h"

// 商品数据结构
USTRUCT(BlueprintType)
struct FShopItemData
{
	GENERATED_BODY()

	// 物品类型（复用现有的EItemType）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	EItemType ItemType = EItemType::None;

	// 商品名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FText DisplayName;

	// 商品描述
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	FText Description;

	// 价格
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 Price = 100;

	// 购买上限（0=无限）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int32 PurchaseLimit = 0;

	// 商品图标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	UTexture2D* Icon = nullptr;
};
