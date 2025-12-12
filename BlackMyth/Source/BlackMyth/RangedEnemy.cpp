#include "RangedEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectileBase.h"
#include "Kismet/GameplayStatics.h"

ARangedEnemy::ARangedEnemy()
{
	// 远程敌人通常移动速度稍慢，或者血量稍低
	// 注意：EnemyBase 会在状态切换时重置 MaxWalkSpeed，所以我们需要设置 PatrollingSpeed 和 ChasingSpeed
	PatrollingSpeed = 200.0f;
	ChasingSpeed = 400.0f;
	
	// 设置默认的射击距离
	RangedAttackDistance = 1000.0f;

	// 关键修正：将 AttackRadius 设置为射程
	// 这样 EnemyBase 的 Tick 逻辑会在距离 <= AttackRadius (1000) 时停止移动并开始攻击
	AttackRadius = RangedAttackDistance;
}

void ARangedEnemy::Attack()
{
	if (CombatTarget == nullptr || IsDead()) return;
	if (IsStunned()) return;

	UE_LOG(LogTemp, Warning, TEXT("[%s] ARangedEnemy::Attack - Starting Ranged Attack"), *GetName());
	EnemyState = EEnemyState::EES_Engaged;

	if (AttackMontage)
	{
		// 播放攻击音效
		if (AttackSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
		}

		// 注意：这里我们不开启 TraceHitboxComponent，因为是远程攻击
		// 投掷物的生成逻辑在 SpawnProjectile() 中，由动画通知调用

		const float Duration = PlayAnimMontage(AttackMontage);
		if (Duration <= 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] ARangedEnemy::Attack - Montage failed to play!"), *GetName());
			AttackEnd();
		}
		else
		{
			// 设置保底计时器
			GetWorldTimerManager().SetTimer(AttackEndTimer, this, &ARangedEnemy::AttackEnd, Duration, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] ARangedEnemy::Attack - No AttackMontage assigned!"), *GetName());
		AttackEnd();
	}
}

void ARangedEnemy::SpawnProjectile()
{
	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] SpawnProjectile - ProjectileClass is NULL!"), *GetName());
		return;
	}

	if (!CombatTarget) return;

	FVector SpawnLocation = GetActorLocation();
	FRotator SpawnRotation = GetActorRotation();

	// 尝试从 Socket 获取位置
	if (GetMesh()->DoesSocketExist(ProjectileSpawnSocket))
	{
		SpawnLocation = GetMesh()->GetSocketLocation(ProjectileSpawnSocket);
	}
	else
	{
		// 如果没有 Socket，就从前方一点的位置生成
		SpawnLocation = GetActorLocation() + GetActorForwardVector() * 50.0f + FVector(0, 0, 50.0f);
	}

	// 计算朝向目标的旋转
	// 简单的预测：直接朝向目标当前位置
	// 进阶：可以计算目标速度进行预判 (Lead Target)
	FVector TargetLocation = CombatTarget->GetActorLocation();
	// 稍微抬高一点目标点，瞄准胸口而不是脚底
	TargetLocation.Z += 50.0f; 

	FVector Direction = (TargetLocation - SpawnLocation).GetSafeNormal();
	SpawnRotation = Direction.Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

	// 视觉效果：隐藏手中的武器 (模拟扔出去了)
	// 假设 CurrentWeapon 是我们手中的武器 Actor
	if (CurrentWeapon)
	{
		CurrentWeapon->SetActorHiddenInGame(true);
		
		// 设置一个计时器，在攻击结束后或者一段时间后让武器重新出现
		// 这里简单起见，直接用一个 Timer，时间稍微比动画短一点，或者在 AttackEnd 里恢复
		FTimerHandle WeaponRespawnTimer;
		GetWorldTimerManager().SetTimer(WeaponRespawnTimer, [this]()
		{
			if (CurrentWeapon)
			{
				CurrentWeapon->SetActorHiddenInGame(false);
			}
		}, 1.0f, false); // 1秒后武器重新出现，您可以根据动画长度调整
	}
}
