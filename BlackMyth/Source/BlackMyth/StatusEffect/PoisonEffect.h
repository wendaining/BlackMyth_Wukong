// 中毒效果 - 持续造成伤害

#pragma once

#include "CoreMinimal.h"
#include "StatusEffectBase.h"
#include "PoisonEffect.generated.h"

// 中毒效果，每隔一定时间对目标造成伤害
UCLASS(Blueprintable, BlueprintType)
class BLACKMYTH_API UPoisonEffect : public UStatusEffectBase
{
	GENERATED_BODY()

public:
	UPoisonEffect();

	// ========== 生命周期重写 ==========

	virtual void OnApplied_Implementation() override;
	virtual void OnTick_Implementation(float DeltaTime) override;
	virtual void OnRemoved_Implementation() override;

protected:
	// ========== 中毒效果属性 ==========

	/** 每秒伤害值 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Poison")
	float DamagePerSecond = 10.0f;

	/** 两次伤害之间的时间间隔 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Poison", meta = (ClampMin = "0.1"))
	float DamageInterval = 0.5f;

	/** 累计时间（用于计算下次伤害） */
	float AccumulatedTime = 0.0f;
};
