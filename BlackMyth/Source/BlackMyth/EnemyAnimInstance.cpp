#include "EnemyAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 获取拥有该动画的角色
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	// 如果角色引用失效，尝试重新获取
	if (OwnerCharacter == nullptr)
	{
		OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	}

	if (OwnerCharacter)
	{
		// 1. 计算速度 (只取水平速度，忽略垂直速度)
		FVector Velocity = OwnerCharacter->GetVelocity();
		Velocity.Z = 0.0f;
		Speed = Velocity.Size();

		// 2. 更新是否移动状态
		bIsMoving = Speed > 3.0f; // 给一点容差，防止浮点数抖动
	}
}
