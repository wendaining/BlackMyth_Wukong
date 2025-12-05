// Hitbox组件 - 近战攻击碰撞检测

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "DamageTypes.h"
#include "HitboxComponent.generated.h"

// 前向声明
class UCombatComponent;
class UHealthComponent;
class AEnemyBase;

// 命中事件委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHitDetected, AActor*, HitActor, const FHitResult&, HitResult);

// Hitbox激活/停用事件
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitboxStateChanged, bool, bIsActive);

/**
 * Hitbox组件
 * 用于检测近战攻击的碰撞，挂载在武器或角色骨骼上
 * 支持调试可视化、命中去重、伤害信息传递
 */
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
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

	// ========== CombatComponent 对接==========

	/** 设置关联的战斗组件 */
	UFUNCTION(BlueprintCallable, Category = "Hitbox")
	void SetCombatComponent(UCombatComponent* InCombatComponent);

	/** 获取关联的战斗组件 */
	UFUNCTION(BlueprintPure, Category = "Hitbox")
	UCombatComponent* GetCombatComponent() const;

	// ========== 攻击类型配置==========

	/** 设置当前攻击是否为重击（供 AnimNotify 调用） */
	UFUNCTION(BlueprintCallable, Category = "Hitbox|Attack")
	void SetHeavyAttack(bool bHeavy) { bIsHeavyAttack = bHeavy; }

	/** 设置当前攻击是否为空中攻击（供 AnimNotify 调用） */
	UFUNCTION(BlueprintCallable, Category = "Hitbox|Attack")
	void SetAirAttack(bool bAir) { bIsAirAttack = bAir; }

	/** 当前是否为重击 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Attack")
	bool IsHeavyAttack() const { return bIsHeavyAttack; }

	/** 当前是否为空中攻击 */
	UFUNCTION(BlueprintPure, Category = "Hitbox|Attack")
	bool IsAirAttack() const { return bIsAirAttack; }

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

	/** 对目标应用伤害（MemberC 重写） */
	void ApplyDamageToTarget(AActor* Target, const FHitResult& HitResult);

	/** 绘制调试形状 */
	void DrawDebugHitbox();

	// ========== 目标有效性检查 ==========

	/** 检查目标是否为有效攻击对象 */
	bool IsValidTarget(AActor* Target) const;

	/** 尝试对 EnemyBase 应用伤害 */
	bool TryApplyDamageToEnemy(AEnemyBase* Enemy, float FinalDamage);

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

	// ========== 攻击类型状态 ==========

	/** 当前攻击是否为重击 */
	UPROPERTY(BlueprintReadWrite, Category = "Hitbox|Attack")
	bool bIsHeavyAttack = false;

	/** 当前攻击是否为空中攻击 */
	UPROPERTY(BlueprintReadWrite, Category = "Hitbox|Attack")
	bool bIsAirAttack = false;

private:
	/** Hitbox 是否激活 */
	bool bIsActive = false;

	/** 本次激活期间已命中的 Actor 列表（防止重复伤害） */
	UPROPERTY()
	TArray<AActor*> HitActors;

	/** 命中闪烁计时器 */
	float HitFlashTimer = 0.0f;

	/** 缓存的战斗组件引用（MemberC 新增） */
	UPROPERTY()
	TWeakObjectPtr<UCombatComponent> CachedCombatComponent;
};
