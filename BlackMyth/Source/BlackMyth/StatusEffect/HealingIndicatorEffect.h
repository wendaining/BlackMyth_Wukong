// 治疗指示效果 - 仅用于显示治疗图标，不修改属性
#pragma once

#include "CoreMinimal.h"
#include "StatusEffectBase.h"
#include "HealingIndicatorEffect.generated.h"

/**
 * 治疗指示效果
 * 用于在UI上显示血药使用后的图标，不实际修改生命值
 * 实际治疗由 HealthComponent::Heal() 完成
 */
UCLASS()
class BLACKMYTH_API UHealingIndicatorEffect : public UStatusEffectBase
{
	GENERATED_BODY()

public:
	UHealingIndicatorEffect();

protected:
	virtual void OnApplied_Implementation() override;
	virtual void OnRemoved_Implementation() override;
};
