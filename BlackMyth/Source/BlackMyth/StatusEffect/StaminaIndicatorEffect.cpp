// 体力恢复指示效果实现

#include "StaminaIndicatorEffect.h"

UStaminaIndicatorEffect::UStaminaIndicatorEffect()
{
	EffectType = EStatusEffectType::StaminaIndicator;
	Duration = 2.0f;  // 显示2秒后消失
	bStackable = false;

	// 禁用视觉效果 - 只显示图标，不发光
	EmissiveIntensity = 0.0f;
}

void UStaminaIndicatorEffect::OnApplied_Implementation()
{
	Super::OnApplied_Implementation();

	AActor* Target = GetOwnerActor();
	if (Target)
	{
		UE_LOG(LogTemp, Log, TEXT("StaminaIndicatorEffect: Applied to %s (display only, stamina was already restored)"),
			*Target->GetName());
	}
}

void UStaminaIndicatorEffect::OnRemoved_Implementation()
{
	AActor* Target = GetOwnerActor();
	if (Target)
	{
		UE_LOG(LogTemp, Log, TEXT("StaminaIndicatorEffect: Removed from %s"), *Target->GetName());
	}

	Super::OnRemoved_Implementation();
}
