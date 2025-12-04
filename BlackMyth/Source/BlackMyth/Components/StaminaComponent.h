// 体力值组件 - 可复用于任何需要体力系统的角色

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StaminaComponent.generated.h"

// 体力值变化委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStaminaChangedSignature, float, CurrentStamina, float, MaxStamina);

// 体力耗尽委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaDepleted);

/**
 * 体力值组件
 * 管理角色的体力消耗与恢复
 * 可挂载到任何 Actor 上
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLACKMYTH_API UStaminaComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UStaminaComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========== 公共接口 ==========

	/** 消耗体力 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void ConsumeStamina(float Amount);

	/** 恢复体力 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void RestoreStamina(float Amount);

	/** 设置是否正在进行持续消耗（如冲刺） */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetContinuousConsumption(bool bEnabled, float CostPerSecond = 0.0f);

	/** 设置是否允许恢复 */
	UFUNCTION(BlueprintCallable, Category = "Stamina")
	void SetCanRegenerate(bool bCanRegen);

	// ========== 查询接口 ==========

	/** 获取当前体力值 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetCurrentStamina() const { return CurrentStamina; }

	/** 获取最大体力值 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetMaxStamina() const { return MaxStamina; }

	/** 获取体力百分比 (0-1) */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	float GetStaminaPercent() const { return MaxStamina > 0.0f ? CurrentStamina / MaxStamina : 0.0f; }

	/** 检查是否有足够体力 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool HasEnoughStamina(float Cost) const { return CurrentStamina >= Cost; }

	/** 体力是否已耗尽 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool IsStaminaDepleted() const { return CurrentStamina <= 0.0f; }

	/** 体力是否已满 */
	UFUNCTION(BlueprintPure, Category = "Stamina")
	bool IsStaminaFull() const { return CurrentStamina >= MaxStamina; }

	// ========== 委托 ==========

	/** 体力值变化时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Stamina|Events")
	FOnStaminaChangedSignature OnStaminaChanged;

	/** 体力耗尽时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Stamina|Events")
	FOnStaminaDepleted OnStaminaDepleted;

	// ========== 可配置属性 ==========

	/** 最大体力值 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Config")
	float MaxStamina = 100.0f;

	/** 体力自动恢复速度（每秒恢复量） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Config")
	float StaminaRegenRate = 15.0f;

	/** 体力恢复延迟（消耗体力后多久开始恢复） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Config")
	float StaminaRegenDelay = 1.0f;

	// ========== 预设消耗值 ==========

	/** 冲刺每秒消耗体力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Costs")
	float SprintStaminaCost = 20.0f;

	/** 跳跃消耗体力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Costs")
	float JumpStaminaCost = 15.0f;

	/** 攻击消耗体力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Costs")
	float AttackStaminaCost = 10.0f;

	/** 翻滚消耗体力 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stamina|Costs")
	float DodgeStaminaCost = 20.0f;

protected:
	/** 当前体力值 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina|Runtime")
	float CurrentStamina;

private:
	// ========== 内部状态 ==========
	
	/** 体力恢复延迟计时器 */
	float StaminaRegenDelayTimer = 0.0f;

	/** 是否正在持续消耗（如冲刺） */
	bool bIsContinuousConsumption = false;

	/** 持续消耗每秒的量 */
	float ContinuousConsumptionRate = 0.0f;

	/** 是否允许恢复 */
	bool bCanRegenerate = true;

	/** 更新体力恢复 */
	void UpdateStaminaRegeneration(float DeltaTime);

	/** 广播体力变化 */
	void BroadcastStaminaChange(float OldValue);
};
