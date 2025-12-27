// 攻击力Buff状态效果

#pragma once

#include "CoreMinimal.h"
#include "StatusEffectBase.h"
#include "AttackBuffEffect.generated.h"

/**
 * 攻击力增益Buff
 * 提升角色攻击力一定百分比，持续一段时间
 */
UCLASS(Blueprintable, BlueprintType)
class BLACKMYTH_API UAttackBuffEffect : public UStatusEffectBase
{
	GENERATED_BODY()

public:
	UAttackBuffEffect();

	virtual void OnApplied_Implementation() override;
	virtual void OnRemoved_Implementation() override;

	/** Buff倍率（1.3 = 提升30%攻击力） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Buff")
	float AttackMultiplier = 1.3f;

private:
	/** 保存原始攻击力加成，用于移除时恢复 */
	float OriginalAttackBonus = 0.0f;
};
