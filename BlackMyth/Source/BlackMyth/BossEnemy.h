#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "BossEnemy.generated.h"

class UBossHealthBar;

/** Boss 阶段枚举 */
UENUM(BlueprintType)
enum class EBossPhase : uint8
{
	Phase1 UMETA(DisplayName = "Phase 1"),
	Phase2 UMETA(DisplayName = "Phase 2"),
};

/**
 * Boss 敌人 (二郎神杨戬)
 */
UCLASS()
class BLACKMYTH_API ABossEnemy : public AEnemyBase
{
	GENERATED_BODY()
	
public:
	ABossEnemy();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** 重写受击函数以处理阶段转换 */
	virtual void ReceiveDamage(float Damage, AActor* DamageInstigator, bool bCanBeDodged = true) override;

	/** 重写死亡函数以调整尸体存留时间 */
	virtual void Die() override;

	/** 激活 Boss (由触发器调用) */
	UFUNCTION(BlueprintCallable, Category = "Boss")
	void ActivateBoss(AActor* Target);

	/** 设置 Boss 血条可见性 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetBossHealthVisibility(bool bVisible);

	/** 执行闪避 */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformDodge();

	/** 召唤哮天犬 (技能) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SummonDog();

	// ========== Boss 属性 ==========

	/** 当前阶段 (公开，供动画蓝图使用) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
	EBossPhase CurrentPhase = EBossPhase::Phase1;

protected:
	/** 检查是否需要进入第二阶段 */
	void CheckPhaseTransition();

	/** 攻击逻辑 (重写以支持随机出招) */
	virtual void Attack() override;

	/** 进入第二阶段 */
	void EnterPhase2();

	/** Boss 血条 UI 类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UBossHealthBar> BossHealthBarClass;

	/** Boss 血条 UI 实例 */
	UPROPERTY()
	UBossHealthBar* BossHealthBarWidget;

	/** 进入第二阶段的血量百分比阈值 (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Config")
	float Phase2Threshold = 0.5f;

	/** 是否处于无敌状态 (闪避或转阶段时) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|State")
	bool bIsInvulnerable = false;

	// ========== 动画资源 ==========

	/** 转阶段蒙太奇 (变身/爆发) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> Phase2TransitionMontage;

	/** 闪避蒙太奇列表 (随机播放) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Animation")
	TArray<UAnimMontage*> DodgeMontages;

	/** 轻攻击连招列表 (Combo A: atk00a, atk00b) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Animation")
	TArray<UAnimMontage*> LightAttackMontages;

	/** 重攻击列表 (Thrust, Overhead: atk01, atk04) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Animation")
	TArray<UAnimMontage*> HeavyAttackMontages;

	/** 召唤哮天犬蒙太奇 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> SummonDogMontage;

	// ========== 二阶段特效 (Phase 2 Visuals) ==========

	/** 二阶段身上持续的粒子特效 (Niagara) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Visuals")
	TObjectPtr<class UNiagaraSystem> Phase2Effect;

	/** 二阶段覆盖材质 (Overlay Material) - 实现金光效果最直接的方法 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Visuals")
	TObjectPtr<class UMaterialInterface> Phase2OverlayMaterial;

	/** 二阶段武器附魔效果 (可选) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Visuals")
	TObjectPtr<class UNiagaraSystem> WeaponEffect;

public:
	/** 执行轻攻击 (随机选择) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformLightAttack();

	/** 执行重攻击 (随机选择) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PerformHeavyAttack();

	/** 哮天犬蓝图类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Skill")
	TSubclassOf<AActor> DogClass;

	/** 哮天犬生成偏移 (相对于二郎神前方) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Skill")
	FVector DogSpawnOffset = FVector(320.0f, 0.0f, 50.0f);

	/** 死亡后尸体消失的时间 (默认 5秒，Boss 可以设长点，如 20秒) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Config")
	float DeathLifeSpan = 20.0f;

	// [Legacy] 竞技场空气墙已迁移至 BossCombatTrigger

	// ========== 内部状态 ==========
	bool bHasEnteredPhase2 = false;
	FTimerHandle DodgeTimer;

	/** 远程攻击频率计时器 (二阶段专用) */
	float RangedAttackCheckTimer = 0.0f;
};
