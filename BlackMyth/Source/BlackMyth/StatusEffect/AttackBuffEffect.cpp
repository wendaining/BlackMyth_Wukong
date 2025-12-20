// 攻击力Buff状态效果实现

#include "AttackBuffEffect.h"
#include "../Components/CombatComponent.h"

UAttackBuffEffect::UAttackBuffEffect()
{
	EffectType = EStatusEffectType::AttackBuff;
	Duration = 10.0f;
	bStackable = false;  // 不可叠加，刷新持续时间

	// 视觉效果配置 - 红色
	TintColor = FLinearColor(1.0f, 0.3f, 0.2f, 1.0f);
	EmissiveColor = FLinearColor(1.0f, 0.2f, 0.0f);
	EmissiveIntensity = 0.01f;
}

void UAttackBuffEffect::OnApplied_Implementation()
{
	Super::OnApplied_Implementation();

	AActor* Target = GetOwnerActor();
	if (!Target) return;

	if (UCombatComponent* Combat = Target->FindComponentByClass<UCombatComponent>())
	{
		// 保存原始攻击力加成
		OriginalAttackBonus = Combat->GetAttackPowerBonus();

		// 计算并应用攻击力提升
		float BaseAttack = Combat->GetBaseAttackPower();
		float BonusIncrease = BaseAttack * (AttackMultiplier - 1.0f);
		Combat->SetAttackPowerBonus(OriginalAttackBonus + BonusIncrease);

		UE_LOG(LogTemp, Log, TEXT("AttackBuffEffect: Applied to %s, BaseAttack=%.1f, Bonus=%.1f -> %.1f"),
			*Target->GetName(), BaseAttack, OriginalAttackBonus, Combat->GetAttackPowerBonus());
	}
}

void UAttackBuffEffect::OnRemoved_Implementation()
{
	AActor* Target = GetOwnerActor();
	if (Target)
	{
		if (UCombatComponent* Combat = Target->FindComponentByClass<UCombatComponent>())
		{
			// 恢复原始攻击力加成
			Combat->SetAttackPowerBonus(OriginalAttackBonus);

			UE_LOG(LogTemp, Log, TEXT("AttackBuffEffect: Removed from %s, Bonus restored to %.1f"),
				*Target->GetName(), OriginalAttackBonus);
		}
	}

	Super::OnRemoved_Implementation();
}
