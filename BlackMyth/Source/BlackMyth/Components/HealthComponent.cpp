// 生命值组件实现

#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 初始化生命值为满
	CurrentHealth = MaxHealth;
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComponent::TakeDamage(float Damage, AActor* Instigator)
{
	// 无敌或已死亡则不受伤害
	if (bIsInvincible || bIsDead || Damage <= 0.0f)
	{
		return;
	}

	// 应用伤害减免
	float FinalDamage = Damage * DamageReductionMultiplier;

	float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - FinalDamage, 0.0f, MaxHealth);

	// 广播受伤事件
	BroadcastHealthChange();
	OnDamageTaken.Broadcast(Damage, Instigator, CurrentHealth);

	// 重置恢复延迟
	HealthRegenDelayTimer = HealthRegenDelay;

	// 检查死亡
	if (CurrentHealth <= 0.0f && !bIsDead)
	{
		bIsDead = true;
		OnDeath.Broadcast(Instigator);
	}
}

void UHealthComponent::Heal(float Amount)
{
	if (bIsDead || Amount <= 0.0f)
	{
		return;
	}

	float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Min(MaxHealth, CurrentHealth + Amount);

	float ActualHeal = CurrentHealth - OldHealth;
	if (ActualHeal > 0.0f)
	{
		OnHealed.Broadcast(ActualHeal, CurrentHealth);
		BroadcastHealthChange();
	}
}

void UHealthComponent::FullHeal()
{
	if (bIsDead)
	{
		return;
	}

	float OldHealth = CurrentHealth;
	CurrentHealth = MaxHealth;

	if (CurrentHealth != OldHealth)
	{
		OnHealed.Broadcast(CurrentHealth - OldHealth, CurrentHealth);
		BroadcastHealthChange();
	}
}

void UHealthComponent::SetHealth(float NewHealth)
{
	float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);

	if (CurrentHealth != OldHealth)
	{
		BroadcastHealthChange();
	}

	// 检查死亡
	if (CurrentHealth <= 0.0f && !bIsDead)
	{
		bIsDead = true;
		OnDeath.Broadcast(nullptr);
	}
}

void UHealthComponent::Revive()
{
	// 重置死亡状态
	bIsDead = false;
	
	// 恢复满血
	CurrentHealth = MaxHealth;
	
	// 广播生命值变化
	BroadcastHealthChange();
	
	UE_LOG(LogTemp, Log, TEXT("[HealthComponent] Character revived with full health: %f/%f"), CurrentHealth, MaxHealth);
}


void UHealthComponent::Kill(AActor* Killer)
{
	if (bIsDead)
	{
		return;
	}

	CurrentHealth = 0.0f;
	bIsDead = true;

	BroadcastHealthChange();
	OnDeath.Broadcast(Killer);
}

void UHealthComponent::SetInvincible(bool bInvincible)
{
	bIsInvincible = bInvincible;
}

void UHealthComponent::BroadcastHealthChange()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}
