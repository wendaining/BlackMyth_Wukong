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

	/** 生成投掷物 (供动画通知调用) */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SpawnProjectile();

protected:
	virtual void Attack() override;

	/** 理想的射击距离 (在这个距离内且有视野就会停止移动) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float RangedAttackDistance = 1000.0f;

	/** 投掷物类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<class AProjectileBase> ProjectileClass;

	/** 投掷物生成插槽 (通常是手部) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName ProjectileSpawnSocket = FName("WeaponSocket"); // 默认用 WeaponSocket，也可以改成 Hand_R
};
