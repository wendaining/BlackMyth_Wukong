// Hitbox组件 - 近战攻击碰撞检测

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "DamageTypes.h"
#include "HitboxComponent.generated.h"

// 命中事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHitDetected, AActor*, HitActor, const FHitResult&, HitResult);

// Hitbox激活/停用事件
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitboxStateChanged, bool, bIsActive);

/**
 * Hitbox组件
 * 用于检测近战攻击的碰撞，挂载在武器或角色骨骼上
 * 支持调试可视化、命中去重、伤害信息传递
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class BLACKMYTH_API UHitboxComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	UHitboxComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ========== 激活控制 ==========

	/** 激活 Hitbox（开始检测碰撞） */
	UFUNCTION(BlueprintCallable, Category = "Hitbox")
	void ActivateHitbox();

	/** 停用 Hitbox（停止检测碰撞） */
	UFUNCTION(BlueprintCallable, Category = "Hitbox")
	void DeactivateHitbox();

	/** 设置 Hitbox 激活状态 */
	UFUNCTION(BlueprintCallable, Category = "Hitbox")
	void SetHitboxActive(bool bActive);

	/** Hitbox 是否激活 */
	UFUNCTION(BlueprintPure, Category = "Hitbox")
	bool IsHitboxActive() const { return bIsActive; }

	/** 清除已命中列表（允许再次命中同一目标） */
	UFUNCTION(BlueprintCallable, Category = "Hitbox")
	void ClearHitActors();

	// ========== 伤害配置 ==========

	/** 设置基础伤害 */
	UFUNCTION(BlueprintCallable, Category = "Hitbox|Damage")
	void SetBaseDamage(float Damage) { DamageInfo.BaseDamage = Damage; }

	/** 设置伤害类型 */
	UFUNCTION(BlueprintCallable, Category = "Hitbox|Damage")
	void SetDamageType(EDamageType Type) { DamageInfo.DamageType = Type; }

	/** 设置攻击类型 */
	UFUNCTION(BlueprintCallable, Category = "Hitbox|Damage")
	void SetAttackType(EAttackType Type) { DamageInfo.AttackType = Type; }

	/** 设置完整伤害信息 */
	UFUNCTION(BlueprintCallable, Category = "Hitbox|Damage")
	void SetDamageInfo(const FDamageInfo& NewDamageInfo) { DamageInfo = NewDamageInfo; }

	/** 获取伤害信息 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Damage")
	FDamageInfo GetDamageInfo() const { return DamageInfo; }

	// ========== 调试 ==========

	/** 设置是否显示调试绘制 */
	UFUNCTION(BlueprintCallable, Category = "Hitbox|Debug")
	void SetDebugDrawEnabled(bool bEnabled) { bDebugDraw = bEnabled; }

	// ========== 委托 ==========

	/** 命中目标时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Hitbox|Events")
	FOnHitDetected OnHitDetected;

	/** Hitbox 状态变化时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Hitbox|Events")
	FOnHitboxStateChanged OnHitboxStateChanged;

protected:
	/** 碰撞开始回调 */
	UFUNCTION()
	void OnHitboxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 对目标应用伤害 */
	void ApplyDamageToTarget(AActor* Target, const FHitResult& HitResult);

	/** 绘制调试形状 */
	void DrawDebugHitbox();

protected:
	// ========== 配置属性 ==========

	/** 默认伤害信息 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox|Damage")
	FDamageInfo DamageInfo;

	/** 是否在编辑器和游戏中绘制调试形状 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox|Debug")
	bool bDebugDraw = true;

	/** 未激活时的调试颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox|Debug")
	FColor DebugColorInactive = FColor::Green;

	/** 激活时的调试颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox|Debug")
	FColor DebugColorActive = FColor::Red;

	/** 命中时的调试颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox|Debug")
	FColor DebugColorHit = FColor::Yellow;

	/** 是否自动对命中目标造成伤害 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	bool bAutoApplyDamage = true;

	/** 是否忽略自己（Owner） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	bool bIgnoreOwner = true;

private:
	/** Hitbox 是否激活 */
	bool bIsActive = false;

	/** 本次激活期间已命中的 Actor 列表（防止重复伤害） */
	UPROPERTY()
	TArray<AActor*> HitActors;

	/** 命中闪烁计时器 */
	float HitFlashTimer = 0.0f;
};
