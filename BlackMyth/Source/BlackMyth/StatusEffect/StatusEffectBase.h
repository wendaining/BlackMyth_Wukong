// 状态效果基类 - 所有状态效果的抽象基类

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "StatusEffectTypes.h"
#include "StatusEffectBase.generated.h"

class UStatusEffectComponent;

// 状态效果基类，所有具体状态效果（中毒、减速、灼烧等）继承此类
UCLASS(Abstract, Blueprintable, BlueprintType)
class BLACKMYTH_API UStatusEffectBase : public UObject
{
	GENERATED_BODY()

public:
	UStatusEffectBase(){};

	// ========== 生命周期方法 ==========

	// 初始化效果
	UFUNCTION(BlueprintCallable, Category = "StatusEffect")
	virtual void Initialize(AActor* InOwner, AActor* InInstigator, float InDuration);

	// 效果被施加时调用
	UFUNCTION(BlueprintNativeEvent, Category = "StatusEffect")
	void OnApplied();
	virtual void OnApplied_Implementation();

	// 每帧更新状态效果
	UFUNCTION(BlueprintNativeEvent, Category = "StatusEffect")
	void OnTick(float DeltaTime);
	virtual void OnTick_Implementation(float DeltaTime);

	// 效果被移除时调用
	UFUNCTION(BlueprintNativeEvent, Category = "StatusEffect")
	void OnRemoved();
	virtual void OnRemoved_Implementation();

	// ========== 查询方法 ==========

	/** 检查效果是否已过期 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	bool IsExpired() const { return RemainingTime <= 0.0f; }

	/** 获取效果类型 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	EStatusEffectType GetEffectType() const { return EffectType; }

	/** 获取剩余时间 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	float GetRemainingTime() const { return RemainingTime; }

	/** 获取总持续时间 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	float GetDuration() const { return Duration; }

	/** 获取剩余时间百分比 (0-1) */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	float GetRemainingTimePercent() const { return Duration > 0.0f ? RemainingTime / Duration : 0.0f; }

	/** 获取效果目标 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	AActor* GetOwnerActor() const { return OwnerActor.Get(); }

	/** 获取效果施加者 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	AActor* GetInstigator() const { return Instigator.Get(); }

	// ========== 时间控制 ==========

	/** 刷新效果持续时间 */
	UFUNCTION(BlueprintCallable, Category = "StatusEffect")
	void RefreshDuration(float NewDuration);

	// ========== 视觉效果 ==========

	/** 获取 Tint 颜色 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect|Visual")
	FLinearColor GetTintColor() const { return TintColor; }

	/** 获取自发光颜色 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect|Visual")
	FLinearColor GetEmissiveColor() const { return EmissiveColor; }

	/** 获取自发光强度 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect|Visual")
	float GetEmissiveIntensity() const { return EmissiveIntensity; }

	/** 是否有视觉效果 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect|Visual")
	bool HasVisualEffect() const { return EmissiveIntensity > 0.0f; }

	/** 是否禁用攻击 */
	UFUNCTION(BlueprintPure, Category = "StatusEffect|Behavior")
	bool IsAttackDisabled() const { return bDisableAttack; }

protected:
	// ========== 效果属性 ==========

	/** 效果类型（子类必须在构造函数中设置） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StatusEffect|Config")
	EStatusEffectType EffectType = EStatusEffectType::None;

	/** 总持续时间 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StatusEffect|Runtime")
	float Duration = 0.0f;

	/** 剩余时间 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StatusEffect|Runtime")
	float RemainingTime = 0.0f;

	/** 是否可叠加（预留接口，当前默认 false） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StatusEffect|Config")
	bool bStackable = false;

	/** 最大叠加层数（预留接口，当前默认 1） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StatusEffect|Config", meta = (EditCondition = "bStackable", ClampMin = "1"))
	int32 MaxStacks = 1;

	/** 是否禁用攻击 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StatusEffect|Behavior")
	bool bDisableAttack = false;

	// ========== 引用 ==========

	/** 效果目标（被施加效果的角色） */
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffect|Runtime")
	TWeakObjectPtr<AActor> OwnerActor;

	/** 效果施加者（造成效果的角色） */
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffect|Runtime")
	TWeakObjectPtr<AActor> Instigator;

	/** 所属的状态效果组件 */
	UPROPERTY(BlueprintReadOnly, Category = "StatusEffect|Runtime")
	TWeakObjectPtr<UStatusEffectComponent> OwnerComponent;

	// ========== 视觉效果属性 ==========

	/** Tint 颜色（叠加到角色材质上的颜色） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StatusEffect|Visual")
	FLinearColor TintColor = FLinearColor::White;

	/** 自发光颜色 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StatusEffect|Visual")
	FLinearColor EmissiveColor = FLinearColor::Black;

	/** 自发光强度 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "StatusEffect|Visual", meta = (ClampMin = "0.0"))
	float EmissiveIntensity = 0.0f;

public:
	/** 设置所属组件 */
	void SetOwnerComponent(UStatusEffectComponent* InComponent);
};
