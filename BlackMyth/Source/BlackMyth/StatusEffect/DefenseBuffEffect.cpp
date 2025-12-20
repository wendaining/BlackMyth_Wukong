// 防御力Buff状态效果实现

#include "DefenseBuffEffect.h"
#include "../Components/HealthComponent.h"

UDefenseBuffEffect::UDefenseBuffEffect()
{
	EffectType = EStatusEffectType::DefenseBuff;
	Duration = 10.0f;
	bStackable = false;  // 不可叠加，刷新持续时间

	// 视觉效果配置 - 金色
	TintColor = FLinearColor(1.0f, 0.8f, 0.2f, 1.0f);
	EmissiveColor = FLinearColor(1.0f, 0.7f, 0.0f);
	EmissiveIntensity = 0.01f;
}

void UDefenseBuffEffect::OnApplied_Implementation()
{
	Super::OnApplied_Implementation();

	AActor* Target = GetOwnerActor();
	if (!Target) return;

	if (UHealthComponent* Health = Target->FindComponentByClass<UHealthComponent>())
	{
		// 设置伤害减免倍率
		Health->DamageReductionMultiplier = DamageReductionMultiplier;

		UE_LOG(LogTemp, Log, TEXT("DefenseBuffEffect: Applied to %s, DamageReduction=%.2f"),
			*Target->GetName(), DamageReductionMultiplier);
	}
}

void UDefenseBuffEffect::OnRemoved_Implementation()
{
	AActor* Target = GetOwnerActor();
	if (Target)
	{
		if (UHealthComponent* Health = Target->FindComponentByClass<UHealthComponent>())
		{
			// 恢复为无减免
			Health->DamageReductionMultiplier = 1.0f;

			UE_LOG(LogTemp, Log, TEXT("DefenseBuffEffect: Removed from %s, DamageReduction restored to 1.0"),
				*Target->GetName());
		}
	}

	Super::OnRemoved_Implementation();
}
