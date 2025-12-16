#include "EnemyDodgeComponent.h"
#include "../EnemyBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "TimerManager.h"

UEnemyDodgeComponent::UEnemyDodgeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// 初始化成员变量
	LastDodgeTime = -100.0f; // 确保游戏开始时可以立即闪避
	bIsInDodge = false;
	OwnerEnemy = nullptr;
}

// 缓存敌人引用
void UEnemyDodgeComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerEnemy = Cast<AEnemyBase>(GetOwner());
	if (!OwnerEnemy)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] EnemyDodgeComponent: Owner is not an EnemyBase!"), *GetOwner()->GetName());
	}
}

// 尝试触发闪避，返回是否成功闪避
bool UEnemyDodgeComponent::TryDodge(const FVector& ThreatDirection)
{
	if (!OwnerEnemy)
	{
		return false;
	}

	// 检查敌人状态：死亡或眩晕时不能闪避
	if (OwnerEnemy->IsDead() || OwnerEnemy->IsStunned())
	{
		return false;
	}

	// 检查是否正在闪避中
	if (bIsInDodge)
	{
		return false;
	}

	// 检查冷却时间
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastDodgeTime < DodgeCooldown)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Dodge on cooldown. Remaining: %.2f s"),
			*OwnerEnemy->GetName(), DodgeCooldown - (CurrentTime - LastDodgeTime));
		return false;
	}

	// 概率判定
	const float RandomValue = FMath::FRand(); // 0.0 ~ 1.0
	if (RandomValue > DodgeChance)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Dodge check failed (Roll: %.2f > Chance: %.2f)"),
			*OwnerEnemy->GetName(), RandomValue, DodgeChance);
		return false;
	}

	// 闪避成功！
	bIsInDodge = true;
	LastDodgeTime = CurrentTime;

	UE_LOG(LogTemp, Warning, TEXT("[%s] Dodge triggered! (Roll: %.2f <= Chance: %.2f)"),
		*OwnerEnemy->GetName(), RandomValue, DodgeChance);

	// 计算闪避方向：垂直于威胁方向（左右随机）
	FVector DodgeDirection;
	const bool bDodgeRight = FMath::RandBool();
	if (bDodgeRight)
	{
		DodgeDirection = FVector::CrossProduct(ThreatDirection, FVector::UpVector).GetSafeNormal();
	}
	else
	{
		DodgeDirection = FVector::CrossProduct(FVector::UpVector, ThreatDirection).GetSafeNormal();
	}

	// 播放闪避音效
	if (DodgeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DodgeSound, OwnerEnemy->GetActorLocation());
	}

	// 播放闪避动画
	float AnimDuration = 0.5f; // 默认持续时间
	if (DodgeMontage)
	{
		AnimDuration = OwnerEnemy->PlayAnimMontage(DodgeMontage);
		UE_LOG(LogTemp, Log, TEXT("[%s] Playing dodge montage: %s (Duration: %.2f)"),
			*OwnerEnemy->GetName(), *DodgeMontage->GetName(), AnimDuration);
	}

	// 应用位移（使用 LaunchCharacter 实现物理闪避）
	const FVector LaunchVelocity = DodgeDirection * DodgeDistance * 2.0f; // 乘以2是因为LaunchCharacter会受重力影响
	OwnerEnemy->LaunchCharacter(LaunchVelocity, true, false);

	// 开启无敌帧
	EnableInvincibility();

	// 设置闪避结束计时器
	if (AnimDuration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			InvincibilityTimer,
			this,
			&UEnemyDodgeComponent::OnDodgeEnd,
			AnimDuration,
			false
		);
	}
	else
	{
		// 如果没有动画，使用默认持续时间
		GetWorld()->GetTimerManager().SetTimer(
			InvincibilityTimer,
			this,
			&UEnemyDodgeComponent::OnDodgeEnd,
			0.5f,
			false
		);
	}

	return true;
}

// 闪避结束回调
void UEnemyDodgeComponent::OnDodgeEnd()
{
	DisableInvincibility();
	bIsInDodge = false;

	UE_LOG(LogTemp, Log, TEXT("[%s] Dodge ended."),
		OwnerEnemy ? *OwnerEnemy->GetName() : TEXT("Unknown"));
}

// 开启无敌帧
void UEnemyDodgeComponent::EnableInvincibility()
{
	// 无敌帧通过 bIsInDodge 标志实现
	// 在 EnemyBase::ReceiveDamage 中，如果 DodgeComponent->TryDodge() 返回 true，
	// 则不会受到伤害，实现了无敌效果

	UE_LOG(LogTemp, Log, TEXT("[%s] Invincibility enabled (Duration: %.2f s)"),
		OwnerEnemy ? *OwnerEnemy->GetName() : TEXT("Unknown"), InvincibilityDuration);

	// 如果需要在特定时间后关闭无敌帧（而不是等动画结束）
	// 可以在这里设置单独的计时器
	// 但当前设计是闪避期间全程无敌，所以不需要
}

// 关闭无敌帧
void UEnemyDodgeComponent::DisableInvincibility()
{
	UE_LOG(LogTemp, Log, TEXT("[%s] Invincibility disabled."),
		OwnerEnemy ? *OwnerEnemy->GetName() : TEXT("Unknown"));
}
