// 体力恢复指示效果 - 仅用于显示体力药图标，不修改属性
#pragma once

#include "CoreMinimal.h"
#include "StatusEffectBase.h"
#include "StaminaIndicatorEffect.generated.h"

/**
 * 体力恢复指示效果
 * 用于在UI上显示体力药使用后的图标，不实际修改体力值
 * 实际体力恢复由 StaminaComponent::RestoreStamina() 完成
 */
UCLASS()
class BLACKMYTH_API UStaminaIndicatorEffect : public UStatusEffectBase
{
	GENERATED_BODY()

public:
	UStaminaIndicatorEffect();

protected:
	// 不需要实现任何实际效果，只是显示图标
	virtual void OnApplied_Implementation() override;
	virtual void OnRemoved_Implementation() override;
};
