#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

/**
 * 敌人的动画实例基类
 * 负责在 C++ 层计算动画所需的变量（如速度），供蓝图使用
 */
UCLASS()
class BLACKMYTH_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

protected:
	// 暴露给蓝图的变量，BlueprintReadOnly 表示蓝图只能读不能写
	
	/** 当前移动速度 */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed;

	/** 是否在移动 */
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsMoving;

	/** 拥有的角色引用 */
	UPROPERTY(BlueprintReadOnly, Category = "References")
	class ACharacter* OwnerCharacter;
};
