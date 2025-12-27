#include "BossAnimInstance.h"
#include "BossEnemy.h"

void UBossAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// 获取 Boss 引用
	Boss = Cast<ABossEnemy>(OwnerCharacter);
}

void UBossAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	// 如果 Boss 引用失效，尝试重新获取
	if (Boss == nullptr && OwnerCharacter)
	{
		Boss = Cast<ABossEnemy>(OwnerCharacter);
	}

	if (Boss)
	{
		// 更新 Boss 特有的状态
		CurrentPhase = Boss->CurrentPhase;
	}
}

