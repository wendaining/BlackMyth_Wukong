#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "RangedEnemy.generated.h"

/**
 * 远程敌人
 * 特性：
 * 1. 攻击范围远
 * 2. 移动逻辑为：进入射程且看到目标即停止
 * 3. 不会进行近战攻击
 */
UCLASS()
class BLACKMYTH_API ARangedEnemy : public AEnemyBase
{
	GENERATED_BODY()
	
public:
	ARangedEnemy();

	/** 获取理想的射击范围 */
	UFUNCTION(BlueprintPure, Category = "AI")
	float GetRangedAttackDistance() const { return RangedAttackDistance; }

protected:
	/** 理想的射击距离 (在这个距离内且有视野就会停止移动) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float RangedAttackDistance = 1000.0f;
};
