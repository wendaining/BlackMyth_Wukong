// 物品系统 - 类型定义和数据结构

#pragma once

#include "CoreMinimal.h"
#include "ItemTypes.generated.h"

// 物品类型枚举
UENUM(BlueprintType)
enum class EItemType : uint8
{
	None         UMETA(DisplayName = "None"),
	HealthPotion UMETA(DisplayName = "Health Potion"),    // 回血药
	StaminaPotion UMETA(DisplayName = "Stamina Potion"),  // 回体力药
	AttackBuff   UMETA(DisplayName = "Attack Buff"),      // 攻击力Buff
	DefenseBuff  UMETA(DisplayName = "Defense Buff"),     // 防御力Buff
};

// 物品槽位数据结构
USTRUCT(BlueprintType)
struct FItemSlot
{
	GENERATED_BODY()

	// 物品类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	EItemType ItemType = EItemType::None;

	// 当前数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 CurrentCount = 0;

	// 最大数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	int32 MaxCount = 10;

	// 效果数值（回复量 或 Buff倍率）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float EffectValue = 0.0f;

	// 效果持续时间（秒，药瓶为0表示瞬时）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float EffectDuration = 0.0f;

	// 物品名称（用于UI显示）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText DisplayName;

	// 物品图标（用于UI显示）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UTexture2D* Icon = nullptr;

	// 是否有物品可用
	bool HasItem() const { return CurrentCount > 0; }

	// 消耗一个物品
	bool Consume()
	{
		if (CurrentCount > 0)
		{
			CurrentCount--;
			return true;
		}
		return false;
	}
};
