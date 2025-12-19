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
	virtual void ReceiveDamage(float Damage, AActor* DamageInstigator) override;

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

	/** 召唤哮天犬蒙太奇 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Animation")
	TObjectPtr<UAnimMontage> SummonDogMontage;

	/** 哮天犬蓝图类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Skill")
	TSubclassOf<AActor> DogClass;

	// ========== 内部状态 ==========
	bool bHasEnteredPhase2 = false;
	FTimerHandle DodgeTimer;
};
