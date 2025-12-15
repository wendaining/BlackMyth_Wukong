#pragma once

#include "CoreMinimal.h"
#include "BlackMythCharacter.h"
#include "Components/WidgetComponent.h"
#include "EnemyBase.generated.h"

class UBehaviorTree;
class AAIController;
class UHealthComponent;
class UCombatComponent;
class UTraceHitboxComponent;
class UEnemyHealthBarWidget;
class UTeamComponent;
class UNiagaraSystem;
class UNiagaraComponent;

/**
 * 敌人状态枚举
 */
UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	EES_Patrolling UMETA(DisplayName = "Patrolling"),
	EES_Chasing UMETA(DisplayName = "Chasing"),
	EES_Attacking UMETA(DisplayName = "Attacking"),
	EES_Engaged UMETA(DisplayName = "Engaged"),
	EES_Stunned UMETA(DisplayName = "Stunned"),
	EES_Frozen UMETA(DisplayName = "Frozen"),  // 定身状态
	EES_Dead UMETA(DisplayName = "Dead"),
	EES_NoState UMETA(DisplayName = "NoState")
};

/**
 * 敌人基类
 * 继承自 ABlackMythCharacter 以复用摄像机等功能（如击杀特写）
 */
UCLASS()
class BLACKMYTH_API AEnemyBase : public ABlackMythCharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	/**
	 * 接收伤害接口
	 * @param Damage 伤害数值
	 * @param DamageInstigator 伤害来源
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void ReceiveDamage(float Damage, AActor* DamageInstigator);

	/** 获取当前生命值 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetCurrentHealth() const;

	/** 获取最大生命值 */
	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetMaxHealth() const;

	/** 获取行为树资源 */
	UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

	/** 开始巡逻 (公开给 AIController 调用) */
	void StartPatrolling();

protected:
	/** 死亡处理 */
	virtual void Die();

	/** 攻击逻辑 */
	virtual void Attack();
	
	/** 攻击结束回调（需要在动画蓝图中通过通知调用） */
	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	/** 检查战斗目标 */
	void CheckCombatTarget();
	
	/** 检查巡逻目标 */
	void CheckPatrolTarget();

	/** 巡逻计时器结束 */
	void PatrolTimerFinished();

	/** 处理死亡回调 */
	UFUNCTION()
	void HandleDeath(AActor* Killer);

	/** 隐藏/显示血条 (预留接口) */
	void HideHealthBar();
	void ShowHealthBar();
	
	/** 追逐目标 */
	void ChaseTarget();
	
	/** 移动到目标 */
	void MoveToTarget(AActor* Target);
	
	/** 选择新的巡逻点 */
	AActor* ChoosePatrolTarget();
	
	/** 启动攻击计时器 */
	void StartAttackTimer();
	
	/** 清除攻击计时器 */
	void ClearAttackTimer();
	
	/** 清除巡逻计时器 */
	void ClearPatrolTimer();

	// 状态判断辅助函数
	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();
	bool IsInsideAttackRadius();
	bool IsChasing();
	bool IsAttacking();
	bool IsEngaged();
	bool InTargetRange(AActor* Target, double Radius);

public:
	bool IsDead();

	// ========== 新增：头顶血条 ==========

	// 血条 Widget 组件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetComponent> HealthBarWidgetComponent;

	// 血条 Widget 类
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UEnemyHealthBarWidget> HealthBarWidgetClass;

	// 血条 Widget 实例引用
	UPROPERTY()
	TObjectPtr<UEnemyHealthBarWidget> HealthBarWidget;

	// 血条距离头顶的高度偏移
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	float HealthBarHeightOffset = 120.0f;

	// 是否始终显示血条（false = 仅战斗时显示）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool bAlwaysShowHealthBar = false;

protected:
	// 基础属性
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	// float MaxHealth = 100.0f; // Moved to HealthComponent

	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	// float CurrentHealth; // Moved to HealthComponent

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTraceHitboxComponent> TraceHitboxComponent;

	/** 阵营组件（用于敌我判定，默认为敌人阵营） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTeamComponent> TeamComponent;

	// 状态
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	UPROPERTY()
	TObjectPtr<AAIController> EnemyController;

	// 战斗目标
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<AActor> CombatTarget;

	// 巡逻目标
	UPROPERTY(EditInstanceOnly, Category = "AI")
	TObjectPtr<AActor> PatrolTarget;

	// 可选的巡逻点列表
	UPROPERTY(EditInstanceOnly, Category = "AI")
	TArray<AActor*> PatrolTargets;

	// AI 参数配置
	UPROPERTY(EditAnywhere, Category = "AI")
	double CombatRadius = 1000.f;

	UPROPERTY(EditAnywhere, Category = "AI")
	double AttackRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = "AI")
	double PatrolRadius = 200.f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float PatrollingSpeed = 125.f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float ChasingSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float PatrolWaitMin = 2.f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float PatrolWaitMax = 5.f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float AttackMin = 0.5f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float AttackMax = 1.0f;

	// 计时器句柄
	FTimerHandle PatrolTimer;
	FTimerHandle AttackTimer;
	FTimerHandle AggroTimer;
	FTimerHandle AttackEndTimer; // 攻击结束计时器（保底机制）

	// 行为树 (保留，以备后续扩展)
	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	/** 
	 * 根据攻击位置播放对应的受击动画 
	 * @param ImpactPoint 攻击者的位置或击中点
	 */
	void PlayHitReactMontage(const FVector& ImpactPoint);

	// 动画蒙太奇 - 受击 (定向)
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> HitReactMontage_Front;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> HitReactMontage_Back;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> HitReactMontage_Left;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> HitReactMontage_Right;

	// 动画蒙太奇 - 死亡
	UPROPERTY(EditDefaultsOnly, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> DeathMontage;

	// 动画蒙太奇 - 发现敌人(咆哮)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> AggroMontage;

	// 动画蒙太奇 - 攻击
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> AttackMontage;

	// 动画蒙太奇 - 眩晕 (Stunned)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation|Combat")
	TObjectPtr<UAnimMontage> StunMontage;

	// ========== 韧性系统 (Poise) ==========

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Poise")
	float MaxPoise = 50.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Poise")
	float CurrentPoise;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Poise")
	float PoiseRecoveryRate = 10.0f; // 每秒恢复量

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Poise")
	float StunDuration = 3.0f; // 眩晕持续时间

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats|Poise")
	float PoiseRecoveryDelay = 3.0f; // 受击后多久开始恢复韧性

	FTimerHandle StunTimer;
	double LastHitTime = 0.0; // 上次受击时间

	// ========== 音效 (SFX) ==========

	// 眩晕时的音效
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> StunSound;

	// 发现敌人时的咆哮声
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> AggroSound;

	// 攻击时的挥舞声/吼叫声
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> AttackSound;

	// 受击时的声音
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> HitSound;

	// 死亡时的声音
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> DeathSound;

	// 攻击命中时的声音 (Impact)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TObjectPtr<USoundBase> AttackImpactSound;

	// ========== 武器系统 (Weapon) ==========

	/** 武器蓝图类 (可选) - 如果设置，将在 BeginPlay 时生成并附加到手中 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
	TSubclassOf<AActor> WeaponClass;

	/** 武器附加的 Socket 名称 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Weapon")
	FName WeaponSocketName = FName("weapon_r");

	/** 当前持有的武器实例 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Weapon")
	TObjectPtr<AActor> CurrentWeapon;

public:
	/** 发现目标时调用 */
	void OnTargetSensed(AActor* Target);

	bool IsStunned();

	// ========== 定身术系统 ==========

	/**
	 * 施加定身效果
	 * @param Duration 定身持续时间（秒）
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Freeze")
	void ApplyFreeze(float Duration);

	/**
	 * 解除定身效果
	 */
	UFUNCTION(BlueprintCallable, Category = "Combat|Freeze")
	void RemoveFreeze();

	/**
	 * 检查是否处于定身状态
	 */
	UFUNCTION(BlueprintPure, Category = "Combat|Freeze")
	bool IsFrozen() const { return bIsFrozen; }

protected:
	/** 咆哮结束，开始追击 */
	void StartChasingAfterAggro();

	/** 眩晕结束 */
	void StunEnd();

	bool bHasAggroed = false;

	// ========== 定身术系统 (内部实现) ==========

	/** 是否处于定身状态 */
	bool bIsFrozen = false;

	/** 定身前的状态（用于解除后恢复） */
	EEnemyState StateBeforeFreeze = EEnemyState::EES_NoState;

	/** 定身时保存的动画播放位置 */
	float FrozenAnimPosition = 0.0f;

	/** 定身前的移动速度 */
	float MovementSpeedBeforeFreeze = 0.0f;

	/** 定身计时器句柄 */
	FTimerHandle FreezeTimer;

	/** 内部定身处理函数 */
	void OnFreezeTimerExpired();

public:
	// ========== 定身术 UI 与特效 ==========

	/** 定身"定"字 Widget 组件（头顶显示） */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|Freeze")
	TObjectPtr<UWidgetComponent> FreezeTextWidgetComponent;

	/** 定身文字 Widget 类（在蓝图中设置） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|Freeze")
	TSubclassOf<UUserWidget> FreezeTextWidgetClass;

	/** 定身"定"字距离头顶的高度偏移 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Freeze")
	float FreezeTextHeightOffset = 180.0f;

	/** 定身时播放的音效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Freeze")
	TObjectPtr<USoundBase> FreezeSound;

	/** 定身解除时播放的音效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Freeze")
	TObjectPtr<USoundBase> UnfreezeSound;

	/** 定身时播放的粒子特效（Niagara） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX|Freeze")
	TObjectPtr<UNiagaraSystem> FreezeEffect;

	/** 定身解除时播放的粒子特效（Niagara） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX|Freeze")
	TObjectPtr<UNiagaraSystem> UnfreezeEffect;

	/** 定身持续特效组件引用（用于解除时销毁） */
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> ActiveFreezeEffectComponent;
};
