// 体力值组件实现

#include "StaminaComponent.h"

UStaminaComponent::UStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStaminaComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 初始化体力值为满
	CurrentStamina = MaxStamina;
	OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
}

void UStaminaComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	float OldStamina = CurrentStamina;

	// 处理持续消耗（如冲刺）
	if (bIsContinuousConsumption && ContinuousConsumptionRate > 0.0f)
	{
		CurrentStamina = FMath::Max(0.0f, CurrentStamina - ContinuousConsumptionRate * DeltaTime);
		StaminaRegenDelayTimer = StaminaRegenDelay;

		// 体力耗尽时触发委托
		if (CurrentStamina <= 0.0f && OldStamina > 0.0f)
		{
			OnStaminaDepleted.Broadcast();
		}
	}
	else
	{
		// 非持续消耗状态：处理恢复
		UpdateStaminaRegeneration(DeltaTime);
	}

	// 广播变化
	BroadcastStaminaChange(OldStamina);
}

void UStaminaComponent::ConsumeStamina(float Amount)
{
	if (Amount <= 0.0f) return;

	float OldStamina = CurrentStamina;
	CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.0f, MaxStamina);

	// 重置恢复延迟计时器
	StaminaRegenDelayTimer = StaminaRegenDelay;

	// 体力耗尽时触发委托
	if (CurrentStamina <= 0.0f && OldStamina > 0.0f)
	{
		OnStaminaDepleted.Broadcast();
	}

	BroadcastStaminaChange(OldStamina);
}

void UStaminaComponent::RestoreStamina(float Amount)
{
	if (Amount <= 0.0f) return;

	float OldStamina = CurrentStamina;
	CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.0f, MaxStamina);

	BroadcastStaminaChange(OldStamina);
}

void UStaminaComponent::SetContinuousConsumption(bool bEnabled, float CostPerSecond)
{
	bIsContinuousConsumption = bEnabled;
	ContinuousConsumptionRate = bEnabled ? CostPerSecond : 0.0f;

	// 开始持续消耗时重置恢复计时器
	if (bEnabled)
	{
		StaminaRegenDelayTimer = StaminaRegenDelay;
	}
}

void UStaminaComponent::SetCanRegenerate(bool bCanRegen)
{
	bCanRegenerate = bCanRegen;
}

void UStaminaComponent::UpdateStaminaRegeneration(float DeltaTime)
{
	if (!bCanRegenerate) return;

	// 处理恢复延迟
	if (StaminaRegenDelayTimer > 0.0f)
	{
		StaminaRegenDelayTimer -= DeltaTime;
		return;
	}

	// 恢复体力
	if (CurrentStamina < MaxStamina)
	{
		CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRegenRate * DeltaTime);
	}
}

void UStaminaComponent::BroadcastStaminaChange(float OldValue)
{
	if (!FMath::IsNearlyEqual(OldValue, CurrentStamina))
	{
		OnStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
}
