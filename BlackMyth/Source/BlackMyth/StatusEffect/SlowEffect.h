// 减速效果 - 降低目标移动速度

#pragma once

#include "CoreMinimal.h"
#include "StatusEffectBase.h"
#include "SlowEffect.generated.h"

// 减速效果，施加时降低目标移动速度，移除时恢复
UCLASS(Blueprintable, BlueprintType)
class BLACKMYTH_API USlowEffect : public UStatusEffectBase
{
	GENERATED_BODY()

public:
	USlowEffect();

	// ========== 生命周期重写 ==========

	virtual void OnApplied_Implementation() override;
	virtual void OnTick_Implementation(float DeltaTime) override;
	virtual void OnRemoved_Implementation() override;

protected:
	// ========== 减速效果属性 ==========

	/** 速度倍率（0.5即速度变为原来的50%） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Slow", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float SpeedMultiplier = 0.5f;

	/** 保存原始行走速度 */
	float OriginalWalkSpeed = 0.0f;

	/** 是否已应用减速（防止重复应用） */
	bool bSpeedModified = false;
};
