// 生命值组件 - 管理角色的生命值、受伤和死亡

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

// 生命值变化委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, CurrentHealth, float, MaxHealth);

// 受伤委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageTaken, float, Damage, AActor*, Instigator, float, RemainingHealth);

// 死亡委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, Killer);

// 治疗委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealed, float, HealAmount, float, NewHealth);

/**
 * 生命值组件
 * 管理角色的生命值、受伤、治疗和死亡
 * 可挂载到任何需要生命值的 Actor 上
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLACKMYTH_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHealthComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ========== 伤害与治疗 ==========

	/** 受到伤害 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamage(float Damage, AActor* Instigator = nullptr);

	/** 治疗 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float Amount);

	/** 完全恢复生命值 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void FullHeal();

	/** 直接设置生命值 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHealth(float NewHealth);

	/** 立即死亡 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Kill(AActor* Killer = nullptr);

	// ========== 查询接口 ==========

	/** 获取当前生命值 */
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	/** 获取最大生命值 */
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	/** 获取生命值百分比 (0-1) */
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }

	/** 是否存活 */
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsAlive() const { return CurrentHealth > 0.0f; }

	/** 是否死亡 */
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return CurrentHealth <= 0.0f; }

	/** 是否满血 */
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsFullHealth() const { return CurrentHealth >= MaxHealth; }

	// ========== 无敌状态 ==========

	/** 设置无敌状态 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetInvincible(bool bInvincible);

	/** 是否无敌 */
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsInvincible() const { return bIsInvincible; }

	// ========== 委托 ==========

	/** 生命值变化时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnHealthChangedSignature OnHealthChanged;

	/** 受伤时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnDamageTaken OnDamageTaken;

	/** 死亡时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnDeath OnDeath;

	/** 治疗时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnHealed OnHealed;

	// ========== 可配置属性 ==========

	/** 最大生命值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Config")
	float MaxHealth = 100.0f;

	/** 是否启用生命值自动恢复 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Regen")
	bool bEnableHealthRegen = false;

	/** 生命值自动恢复速度（每秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Regen", meta = (EditCondition = "bEnableHealthRegen"))
	float HealthRegenRate = 5.0f;

	/** 恢复延迟（受伤后多久开始恢复） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health|Regen", meta = (EditCondition = "bEnableHealthRegen"))
	float HealthRegenDelay = 3.0f;

protected:
	/** 当前生命值 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health|Runtime")
	float CurrentHealth;

private:
	/** 是否无敌 */
	bool bIsInvincible = false;

	/** 是否已死亡（防止重复触发死亡） */
	bool bIsDead = false;

	/** 恢复延迟计时器 */
	float HealthRegenDelayTimer = 0.0f;

	/** 广播生命值变化 */
	void BroadcastHealthChange();
};
