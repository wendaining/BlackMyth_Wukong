#pragma once

#include "CoreMinimal.h"
#include "EnemyAnimInstance.h"
#include "BossEnemy.h"
#include "BossAnimInstance.generated.h"

/**
 * Boss 专用的动画实例类
 * 继承自 EnemyAnimInstance，添加 Boss 特有的动画变量
 */
UCLASS()
class BLACKMYTH_API UBossAnimInstance : public UEnemyAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

protected:
	/** Boss 的引用 */
	UPROPERTY(BlueprintReadOnly, Category = "Reference")
	class ABossEnemy* Boss;

	/** 当前阶段 (用于动画蓝图切换不同阶段的动画) */
	UPROPERTY(BlueprintReadOnly, Category = "Boss")
	EBossPhase CurrentPhase = EBossPhase::Phase1;
};

