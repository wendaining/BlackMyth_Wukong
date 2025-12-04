// 战斗属性组件实现

#include "CombatComponent.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 确保连击倍率数组至少有 MaxComboCount 个元素
	while (ComboMultipliers.Num() < MaxComboCount)
	{
		ComboMultipliers.Add(1.0f);
	}
}

float UCombatComponent::CalculateDamage(bool bIsHeavyAttack, bool bIsAirAttack, int32 ComboIndex) const
{
	// 基础伤害 = 总攻击力
	float FinalDamage = GetTotalAttackPower();

	// 应用攻击类型倍率
	if (bIsAirAttack)
	{
		FinalDamage *= AirAttackMultiplier;
	}
	else if (bIsHeavyAttack)
	{
		FinalDamage *= HeavyAttackMultiplier;
	}
	else
	{
		FinalDamage *= LightAttackMultiplier;
	}

	// 应用连击倍率
	if (ComboMultipliers.IsValidIndex(ComboIndex))
	{
		FinalDamage *= ComboMultipliers[ComboIndex];
	}

	return FinalDamage;
}

float UCombatComponent::CalculateDamageWithCrit(bool bIsHeavyAttack, bool bIsAirAttack, int32 ComboIndex, bool& OutIsCritical) const
{
	float Damage = CalculateDamage(bIsHeavyAttack, bIsAirAttack, ComboIndex);

	// 判断是否暴击
	OutIsCritical = FMath::FRand() < CriticalChance;
	if (OutIsCritical)
	{
		Damage *= CriticalMultiplier;
	}

	return Damage;
}

void UCombatComponent::SetCurrentComboIndex(int32 NewIndex)
{
	CurrentComboIndex = FMath::Clamp(NewIndex, 0, MaxComboCount - 1);
}

void UCombatComponent::AdvanceCombo()
{
	CurrentComboIndex = (CurrentComboIndex + 1) % MaxComboCount;
}

void UCombatComponent::ResetCombo()
{
	CurrentComboIndex = 0;
}
