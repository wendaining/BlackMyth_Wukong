// 状态效果类型定义 - 包含枚举和配置结构体

#pragma once

#include "CoreMinimal.h"
#include "StatusEffectTypes.generated.h"

// 前向声明
class UStatusEffectBase;

// 状态效果类型枚举
UENUM(BlueprintType)
enum class EStatusEffectType : uint8
{
	None        UMETA(DisplayName = "None"),
	Poison      UMETA(DisplayName = "Poison"),      // 中毒：持续伤害
	Slow        UMETA(DisplayName = "Slow"),        // 减速：降低移动速度
	Burn        UMETA(DisplayName = "Burn"),        // 灼烧：持续火焰伤害（预留）
};

// 状态效果配置结构体，用于在敌人蓝图中配置攻击附带的状态效果
USTRUCT(BlueprintType)
struct FStatusEffectConfig
{
	GENERATED_BODY()

	/** 效果类（需要是 UStatusEffectBase 的子类） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect")
	TSubclassOf<UStatusEffectBase> EffectClass;

	/** 效果持续时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect", meta = (ClampMin = "0.1"))
	float Duration = 5.0f;

	/** 触发概率 (0.0 - 1.0)，1.0 表示必定触发 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StatusEffect", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TriggerChance = 1.0f;

	/** 默认构造函数 */
	FStatusEffectConfig(): EffectClass(nullptr), Duration(5.0f), TriggerChance(1.0f){}

	/** 带参数构造函数 */
	FStatusEffectConfig(TSubclassOf<UStatusEffectBase> InEffectClass, float InDuration, float InTriggerChance = 1.0f)
		: EffectClass(InEffectClass), Duration(InDuration), TriggerChance(InTriggerChance){}
};
