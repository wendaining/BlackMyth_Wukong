// 防御力Buff状态效果

#pragma once

#include "CoreMinimal.h"
#include "StatusEffectBase.h"
#include "DefenseBuffEffect.generated.h"

/**
 * 防御力增益Buff
 * 降低角色受到的伤害一定百分比，持续一段时间
 */
UCLASS(Blueprintable, BlueprintType)
class BLACKMYTH_API UDefenseBuffEffect : public UStatusEffectBase
{
	GENERATED_BODY()

public:
	UDefenseBuffEffect();

	virtual void OnApplied_Implementation() override;
	virtual void OnRemoved_Implementation() override;

	/** 伤害减免倍率（0.5 = 减免50%伤害） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Buff")
	float DamageReductionMultiplier = 0.5f;
};
