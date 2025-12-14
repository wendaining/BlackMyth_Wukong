// 悟空分身类 - 影分身技能生成的AI控制角色

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WukongClone.generated.h"

class UHealthComponent;
class UCombatComponent;
class UTeamComponent;
class UTraceHitboxComponent;

/**
 * 悟空分身
 * 由AI控制，自动攻击附近敌人
 * 有限生命周期后自动消失
 */
UCLASS()
class BLACKMYTH_API AWukongClone : public ACharacter
{
	GENERATED_BODY()

public:
	AWukongClone();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	/** 初始化分身（由召唤者调用） */
	UFUNCTION(BlueprintCallable, Category = "Clone")
	void InitializeClone(AActor* InOwner, float InLifetime = 20.0f);

	/** 执行攻击 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformAttack();

	/** 获取召唤者 */
	UFUNCTION(BlueprintPure, Category = "Clone")
	AActor* GetCloneOwner() const { return CloneOwner; }

	/** 分身消失（播放消失特效后销毁） */
	UFUNCTION(BlueprintCallable, Category = "Clone")
	void Disappear();

protected:
	/** 生命周期结束回调 */
	void OnLifetimeExpired();

	/** 死亡事件回调（绑定到HealthComponent的OnDeath） */
	UFUNCTION()
	void HandleDeath(AActor* Killer);

	/** 寻找最近的敌人 */
	AActor* FindNearestEnemy();

	/** 移动到目标位置 */
	void MoveToTarget(AActor* Target);

protected:
	// ========== 组件 ==========

	/** 生命组件（分身也可以被打死） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComponent;

	/** 战斗组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCombatComponent> CombatComponent;

	/** 阵营组件（用于敌我判定，默认为玩家阵营） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTeamComponent> TeamComponent;

	/** 武器碰撞检测组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTraceHitboxComponent> WeaponTraceHitbox;

	// ========== 配置 ==========

	/** 分身存活时间（秒） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clone|Config")
	float Lifetime = 20.0f;

	/** 攻击范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clone|Config")
	float AttackRange = 200.0f;

	/** 检测敌人范围 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clone|Config")
	float DetectionRange = 1500.0f;

	/** 攻击冷却时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clone|Config")
	float AttackCooldown = 1.5f;

	/** 移动速度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clone|Config")
	float MoveSpeed = 500.0f;

	// ========== 动画 ==========

	/** 攻击蒙太奇数组 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TArray<TObjectPtr<UAnimMontage>> AttackMontages;

	/** 消失特效（可选） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TObjectPtr<UParticleSystem> DisappearEffect;

	/** 消失声音（可选） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TObjectPtr<USoundBase> DisappearSound;

private:
	/** 召唤者引用 */
	UPROPERTY()
	TObjectPtr<AActor> CloneOwner;

	/** 当前目标 */
	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;

	/** 生命周期计时器句柄 */
	FTimerHandle LifetimeTimerHandle;

	/** 攻击冷却计时器 */
	float AttackCooldownTimer = 0.0f;

	/** 当前攻击索引（用于连招） */
	int32 CurrentAttackIndex = 0;

	/** 是否正在攻击 */
	bool bIsAttacking = false;

	/** 是否已初始化 */
	bool bIsInitialized = false;
};
