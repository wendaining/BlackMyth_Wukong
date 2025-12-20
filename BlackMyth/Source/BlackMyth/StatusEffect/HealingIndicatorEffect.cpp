// 治疗指示效果实现

#include "HealingIndicatorEffect.h"

UHealingIndicatorEffect::UHealingIndicatorEffect()
{
	EffectType = EStatusEffectType::HealingIndicator;
	Duration = 2.0f;  // 显示2秒后消失
	bStackable = false;

	// 禁用视觉效果 - 只显示图标，不发光
	EmissiveIntensity = 0.0f;
}

void UHealingIndicatorEffect::OnApplied_Implementation()
{
	Super::OnApplied_Implementation();

	AActor* Target = GetOwnerActor();
	if (Target)
	{
		UE_LOG(LogTemp, Log, TEXT("HealingIndicatorEffect: Applied to %s (display only, healing was already done)"),
			*Target->GetName());
	}
}

void UHealingIndicatorEffect::OnRemoved_Implementation()
{
	AActor* Target = GetOwnerActor();
	if (Target)
	{
		UE_LOG(LogTemp, Log, TEXT("HealingIndicatorEffect: Removed from %s"), *Target->GetName());
	}

	Super::OnRemoved_Implementation();
}
