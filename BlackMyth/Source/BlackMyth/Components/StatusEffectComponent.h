// 状态效果管理组件 - 管理角色身上所有激活的状态效果

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../StatusEffect/StatusEffectTypes.h"
#include "StatusEffectComponent.generated.h"

class UStatusEffectBase;
class USkeletalMeshComponent;
class UMaterialInstanceDynamic;

// ========== 委托声明 ==========

/** 效果施加时广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatusEffectApplied, EStatusEffectType, EffectType, float, Duration);

/** 效果移除时广播 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusEffectRemoved, EStatusEffectType, EffectType);

/** 效果更新时广播（每帧） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatusEffectUpdated, EStatusEffectType, EffectType, float, RemainingTime);

// 将状态效果挂载到角色上，管理所有激活的状态效果
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLACKMYTH_API UStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatusEffectComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========== 效果管理接口 ==========

	// 施加状态效果
	UFUNCTION(BlueprintCallable, Category = "StatusEffect")
	UStatusEffectBase* ApplyEffect(TSubclassOf<UStatusEffectBase> EffectClass, AActor* InInstigator, float Duration);

	// 移除指定类型的效果
	UFUNCTION(BlueprintCallable, Category = "StatusEffect")
	bool RemoveEffect(EStatusEffectType EffectType);

	// 移除所有效果
	UFUNCTION(BlueprintCallable, Category = "StatusEffect")
	void RemoveAllEffects();

	// 检查是否有指定类型的效果
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	bool HasEffect(EStatusEffectType EffectType) const;

	// 获取指定类型的效果实例
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	UStatusEffectBase* GetEffect(EStatusEffectType EffectType) const;

	// 获取所有激活的效果
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	TArray<UStatusEffectBase*> GetActiveEffects() const { return ActiveEffects; }

	// 获取激活效果数量
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	int32 GetActiveEffectCount() const { return ActiveEffects.Num(); }

	// 检查是否有任何效果禁用了攻击
	UFUNCTION(BlueprintPure, Category = "StatusEffect")
	bool IsAttackDisabled() const;

	// ========== 委托 ==========

	/** 效果施加时广播 */
	UPROPERTY(BlueprintAssignable, Category = "StatusEffect|Events")
	FOnStatusEffectApplied OnEffectApplied;

	/** 效果移除时广播 */
	UPROPERTY(BlueprintAssignable, Category = "StatusEffect|Events")
	FOnStatusEffectRemoved OnEffectRemoved;

	/** 效果更新时广播 */
	UPROPERTY(BlueprintAssignable, Category = "StatusEffect|Events")
	FOnStatusEffectUpdated OnEffectUpdated;

protected:
	// ========== 内部数据 ==========

	/** 当前激活的效果列表 */
	UPROPERTY()
	TArray<UStatusEffectBase*> ActiveEffects;

	/** 缓存的骨骼网格体组件（用于材质修改） */
	UPROPERTY()
	TWeakObjectPtr<USkeletalMeshComponent> CachedMesh;

	/** 动态材质实例列表 */
	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> DynamicMaterials;

	/** 原始材质备份 */
	UPROPERTY()
	TMap<int32, UMaterialInterface*> OriginalMaterials;

	/** 是否已初始化动态材质 */
	bool bMaterialsInitialized = false;

	// ========== 内部方法 ==========

	/** 初始化动态材质 */
	void SetupDynamicMaterials();

	/** 更新材质视觉效果（合并所有激活效果的颜色） */
	void UpdateMaterialEffects();

	/** 重置材质到原始状态 */
	void ResetMaterialEffects();

	/** 内部移除效果（不广播事件） */
	void RemoveEffectInternal(int32 Index);
};
