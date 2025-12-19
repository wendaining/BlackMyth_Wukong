// 中毒效果实现

#include "PoisonEffect.h"
#include "../Components/HealthComponent.h"
#include "GameFramework/Actor.h"

UPoisonEffect::UPoisonEffect()
{
	// 设置效果类型
	EffectType = EStatusEffectType::Poison;

	// 设置视觉效果 - 绿色
	TintColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);
	EmissiveColor = FLinearColor(0.0f, 1.0f, 0.0f);
	EmissiveIntensity =0.01f;

	// 默认伤害配置
	DamagePerSecond = 10.0f;
	DamageInterval = 0.5f;
	AccumulatedTime = 0.0f;
}

void UPoisonEffect::OnApplied_Implementation()
{
	Super::OnApplied_Implementation();

	// 重置累计时间
	AccumulatedTime = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("[PoisonEffect] Applied - DPS: %.1f, Interval: %.1f"),
		DamagePerSecond, DamageInterval);
}

void UPoisonEffect::OnTick_Implementation(float DeltaTime)
{
	// 调用基类更新剩余时间
	Super::OnTick_Implementation(DeltaTime);

	// 累计时间
	AccumulatedTime += DeltaTime;

	// 检查是否到达伤害间隔
	if (AccumulatedTime >= DamageInterval)
	{
		// 计算本次伤害
		float DamageThisTick = DamagePerSecond * DamageInterval;

		// 获取目标的 HealthComponent 并造成伤害
		AActor* Owner = OwnerActor.Get();
		if (Owner)
		{
			UHealthComponent* HealthComp = Owner->FindComponentByClass<UHealthComponent>();
			if (HealthComp)
			{
				HealthComp->TakeDamage(DamageThisTick, Instigator.Get());

				UE_LOG(LogTemp, Verbose, TEXT("[PoisonEffect] Dealt %.1f damage to %s"),
					DamageThisTick, *Owner->GetName());
			}
		}

		// 重置累计时间
		AccumulatedTime = 0.0f;
	}
}

void UPoisonEffect::OnRemoved_Implementation()
{
	Super::OnRemoved_Implementation();

	UE_LOG(LogTemp, Log, TEXT("[PoisonEffect] Removed"));
}
