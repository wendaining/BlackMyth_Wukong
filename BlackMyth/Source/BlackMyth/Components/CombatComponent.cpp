// 战斗属性组件实现

#include "CombatComponent.h"
#include "Engine/World.h"

UCombatComponent::UCombatComponent()
{
	// 启用 Tick 用于检测连击超时
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// 确保连击倍率数组至少有 MaxComboCount 个元素
	while (ComboMultipliers.Num() < MaxComboCount)
	{
		ComboMultipliers.Add(1.0f);
	}

	// 初始化时间戳
	LastAttackTime = 0.0f;
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 检查连击是否超时
	CheckComboTimeout();
}

// ========== 伤害计算 ==========

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

float UCombatComponent::CalculateFinalDamage(FDamageInfo& OutDamageInfo, bool bIsHeavyAttack, bool bIsAirAttack)
{
	// 计算带暴击的伤害
	bool bIsCritical = false;
	float FinalDamage = CalculateDamageWithCrit(bIsHeavyAttack, bIsAirAttack, CurrentComboIndex, bIsCritical);

	// 填充伤害信息结构体
	OutDamageInfo.BaseDamage = FinalDamage;
	OutDamageInfo.bIsCritical = bIsCritical;
	OutDamageInfo.CriticalMultiplier = CriticalMultiplier;
	OutDamageInfo.Instigator = GetOwner();
	OutDamageInfo.DamageCauser = GetOwner();
	OutDamageInfo.ComboIndex = CurrentComboIndex;

	// 设置攻击类型
	if (bIsAirAttack)
	{
		OutDamageInfo.AttackType = EAttackType::Air;
	}
	else if (bIsHeavyAttack)
	{
		OutDamageInfo.AttackType = EAttackType::Heavy;
	}
	else
	{
		OutDamageInfo.AttackType = EAttackType::Light;
	}

	// 根据连击段数设置硬直和击退
	// 最后一段连击（终结技）有更强的效果
	bool bIsFinisher = (CurrentComboIndex == MaxComboCount - 1);
	if (bIsFinisher)
	{
		OutDamageInfo.HitStunDuration = FinisherHitStunDuration;
		OutDamageInfo.KnockbackForce = FinisherKnockbackForce;
	}
	else
	{
		OutDamageInfo.HitStunDuration = NormalHitStunDuration;
		OutDamageInfo.KnockbackForce = 0.0f;
	}

	// 默认可被格挡和闪避
	OutDamageInfo.bCanBeBlocked = true;
	OutDamageInfo.bCanBeDodged = true;

	UE_LOG(LogTemp, Log, TEXT("[CombatComponent] CalculateFinalDamage: %.1f (Crit: %s, Combo: %d/%d)"),
		FinalDamage, bIsCritical ? TEXT("YES") : TEXT("NO"), CurrentComboIndex + 1, MaxComboCount);

	return FinalDamage;
}

// ========== 连击管理 ==========

void UCombatComponent::SetCurrentComboIndex(int32 NewIndex)
{
	int32 OldIndex = CurrentComboIndex;
	CurrentComboIndex = FMath::Clamp(NewIndex, 0, MaxComboCount - 1);

	// 如果索引变化，广播事件
	if (OldIndex != CurrentComboIndex)
	{
		OnComboChanged.Broadcast(CurrentComboIndex);
	}
}

void UCombatComponent::AdvanceCombo()
{
	int32 OldIndex = CurrentComboIndex;
	CurrentComboIndex = (CurrentComboIndex + 1) % MaxComboCount;

	// 更新攻击时间戳
	RecordAttackTime();

	// 广播连击变化
	OnComboChanged.Broadcast(CurrentComboIndex);

	UE_LOG(LogTemp, Log, TEXT("[CombatComponent] AdvanceCombo: %d -> %d"), OldIndex, CurrentComboIndex);
}

void UCombatComponent::ResetCombo()
{
	if (CurrentComboIndex != 0 || bComboWindowOpen)
	{
		CurrentComboIndex = 0;
		bComboWindowOpen = false;

		// 广播连击重置
		OnComboReset.Broadcast();

		UE_LOG(LogTemp, Log, TEXT("[CombatComponent] Combo Reset"));
	}
}

// ========== 连击窗口管理 ==========

void UCombatComponent::OpenComboWindow()
{
	if (!bComboWindowOpen)
	{
		bComboWindowOpen = true;
		UE_LOG(LogTemp, Verbose, TEXT("[CombatComponent] Combo Window Opened"));
	}
}

void UCombatComponent::CloseComboWindow()
{
	if (bComboWindowOpen)
	{
		bComboWindowOpen = false;
		UE_LOG(LogTemp, Verbose, TEXT("[CombatComponent] Combo Window Closed"));
	}
}

bool UCombatComponent::CanContinueCombo() const
{
	// 检查连击窗口是否开启
	if (!bComboWindowOpen)
	{
		return false;
	}

	// 检查是否达到最大连击数
	if (CurrentComboIndex >= MaxComboCount - 1)
	{
		return false;
	}

	// 检查是否超时
	if (GetWorld())
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		if (LastAttackTime > 0.0f && (CurrentTime - LastAttackTime) > ComboResetTime)
		{
			return false;
		}
	}

	return true;
}

void UCombatComponent::RecordAttackTime()
{
	if (GetWorld())
	{
		LastAttackTime = GetWorld()->GetTimeSeconds();
	}
}

void UCombatComponent::CheckComboTimeout()
{
	// 如果当前有连击进行中，检查是否超时
	if (CurrentComboIndex > 0 || bComboWindowOpen)
	{
		if (GetWorld() && LastAttackTime > 0.0f)
		{
			float CurrentTime = GetWorld()->GetTimeSeconds();
			if ((CurrentTime - LastAttackTime) > ComboResetTime)
			{
				ResetCombo();
			}
		}
	}
}
