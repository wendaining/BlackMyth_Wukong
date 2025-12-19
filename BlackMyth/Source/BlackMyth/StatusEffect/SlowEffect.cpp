// 减速效果实现

#include "SlowEffect.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USlowEffect::USlowEffect()
{
	// 设置效果类型
	EffectType = EStatusEffectType::Slow;

	// 设置视觉效果 - 蓝色
	TintColor = FLinearColor(0.3f, 0.5f, 1.0f, 1.0f);
	EmissiveColor = FLinearColor(0.0f, 0.5f, 1.0f);
	EmissiveIntensity = 0.01f;

	// 默认减速配置
	SpeedMultiplier = 0.5f;
	OriginalWalkSpeed = 0.0f;
	bSpeedModified = false;
}

void USlowEffect::OnApplied_Implementation()
{
	Super::OnApplied_Implementation();

	// 获取角色的移动组件
	AActor* Owner = OwnerActor.Get();
	if (!Owner)
	{
		return;
	}

	ACharacter* Character = Cast<ACharacter>(Owner);
	if (!Character)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SlowEffect] Owner is not a Character, cannot apply slow!"));
		return;
	}

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (!MoveComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SlowEffect] Character has no MovementComponent!"));
		return;
	}

	// 保存原始速度
	OriginalWalkSpeed = MoveComp->MaxWalkSpeed;

	// 应用减速
	MoveComp->MaxWalkSpeed *= SpeedMultiplier;
	bSpeedModified = true;

	UE_LOG(LogTemp, Log, TEXT("[SlowEffect] Applied - Original Speed: %.1f, New Speed: %.1f (%.0f%% slow)"),
		OriginalWalkSpeed, MoveComp->MaxWalkSpeed, (1.0f - SpeedMultiplier) * 100.0f);
}

void USlowEffect::OnTick_Implementation(float DeltaTime)
{
	// 调用基类更新剩余时间
	Super::OnTick_Implementation(DeltaTime);
}

void USlowEffect::OnRemoved_Implementation()
{
	// 恢复原始速度
	if (bSpeedModified)
	{
		AActor* Owner = OwnerActor.Get();
		if (Owner)
		{
			ACharacter* Character = Cast<ACharacter>(Owner);
			if (Character)
			{
				UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
				if (MoveComp)
				{
					MoveComp->MaxWalkSpeed = OriginalWalkSpeed;

					UE_LOG(LogTemp, Log, TEXT("[SlowEffect] Removed - Speed restored to: %.1f"),
						OriginalWalkSpeed);
				}
			}
		}

		bSpeedModified = false;
	}

	Super::OnRemoved_Implementation();
}
