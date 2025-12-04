// 战斗属性组件 - 管理攻击力、伤害倍率等战斗相关属性

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

// 伤害事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageDealt, float, Damage, AActor*, Target, bool, bIsCritical);

// 受伤事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageReceived, float, Damage, AActor*, Instigator, float, RemainingHealth);

/**
 * 战斗属性组件
 * 管理角色的攻击力、伤害倍率等战斗属性
 * 可挂载到任何需要战斗能力的 Actor 上
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLACKMYTH_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();

protected:
	virtual void BeginPlay() override;

public:
	// ========== 伤害计算 ==========

	/** 计算最终伤害值 */
	UFUNCTION(BlueprintPure, Category = "Combat|Damage")
	float CalculateDamage(bool bIsHeavyAttack = false, bool bIsAirAttack = false, int32 ComboIndex = 0) const;

	/** 计算带暴击的伤害 */
	UFUNCTION(BlueprintPure, Category = "Combat|Damage")
	float CalculateDamageWithCrit(bool bIsHeavyAttack, bool bIsAirAttack, int32 ComboIndex, bool& OutIsCritical) const;

	// ========== 连击管理 ==========

	/** 获取当前连击索引 */
	UFUNCTION(BlueprintPure, Category = "Combat|Combo")
	int32 GetCurrentComboIndex() const { return CurrentComboIndex; }

	/** 设置当前连击索引 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
	void SetCurrentComboIndex(int32 NewIndex);

	/** 增加连击索引（自动循环） */
	UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
	void AdvanceCombo();

	/** 重置连击 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Combo")
	void ResetCombo();

	/** 获取最大连击数 */
	UFUNCTION(BlueprintPure, Category = "Combat|Combo")
	int32 GetMaxComboCount() const { return MaxComboCount; }

	// ========== 查询接口 ==========

	/** 获取基础攻击力 */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetBaseAttackPower() const { return BaseAttackPower; }

	/** 设置基础攻击力 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetBaseAttackPower(float NewValue) { BaseAttackPower = NewValue; }

	/** 获取攻击力加成（来自装备、Buff等） */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetAttackPowerBonus() const { return AttackPowerBonus; }

	/** 设置攻击力加成 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetAttackPowerBonus(float NewValue) { AttackPowerBonus = NewValue; }

	/** 获取总攻击力（基础 + 加成） */
	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetTotalAttackPower() const { return BaseAttackPower + AttackPowerBonus; }

	// ========== 委托 ==========

	/** 造成伤害时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnDamageDealt OnDamageDealt;

	/** 受到伤害时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnDamageReceived OnDamageReceived;

	// ========== 基础属性 ==========

	/** 基础攻击力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float BaseAttackPower = 10.0f;

	/** 攻击力加成（来自装备、Buff等） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
	float AttackPowerBonus = 0.0f;

	// ========== 伤害倍率 ==========

	/** 轻击伤害倍率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Multipliers")
	float LightAttackMultiplier = 1.0f;

	/** 重击伤害倍率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Multipliers")
	float HeavyAttackMultiplier = 2.0f;

	/** 空中攻击伤害倍率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Multipliers")
	float AirAttackMultiplier = 1.8f;

	/** 连击各段伤害倍率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Multipliers")
	TArray<float> ComboMultipliers = {1.0f, 1.2f, 1.5f};

	// ========== 暴击属性 ==========

	/** 暴击率 (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Critical", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CriticalChance = 0.05f;

	/** 暴击伤害倍率 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Critical")
	float CriticalMultiplier = 1.5f;

	// ========== 连击设置 ==========

	/** 最大连击数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Combo")
	int32 MaxComboCount = 3;

	/** 连击重置时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Combo")
	float ComboResetTime = 1.0f;

protected:
	/** 当前连击索引 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Runtime")
	int32 CurrentComboIndex = 0;
};
