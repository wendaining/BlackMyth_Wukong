#include "BossEnemy.h"
#include "BossHealthBar.h"
#include "Kismet/GameplayStatics.h"
#include "Components/HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/EnemyDodgeComponent.h"
#include "Combat/TraceHitboxComponent.h" // [Fix] Include Hitbox Component

// ... (Existing Constructor and other functions unchanged) ...

void ABossEnemy::PerformLightAttack()
{
	if (IsDead() || IsStunned() || LightAttackMontages.Num() == 0)
	{
		AttackEnd();
		return;
	}

	int32 Index = FMath::RandRange(0, LightAttackMontages.Num() - 1);
	UAnimMontage* MontageToPlay = LightAttackMontages[Index];

	if (MontageToPlay)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Performing Light Attack: %s"), *GetName(), *MontageToPlay->GetName());
		
		if (CombatTarget)
		{
			FVector Direction = (CombatTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			Direction.Z = 0.f;
			SetActorRotation(Direction.Rotation());
		}

		// [Fix] 强制开启攻击判定 (因为手动创建的蒙太奇可能缺少 Notify)
		if (TraceHitboxComponent)
		{
			TraceHitboxComponent->ActivateTrace();
		}

		float Duration = PlayAnimMontage(MontageToPlay);
		bIsInvulnerable = false; 

		if (Duration > 0.f)
		{
			// 使用 Lambda 表达式来调用受保护的父类函数
			GetWorldTimerManager().SetTimer(AttackEndTimer, [this]()
			{
				// [Fix] 攻击结束时关闭判定
				if (TraceHitboxComponent) TraceHitboxComponent->DeactivateTrace();
				this->AttackEnd();
			}, Duration, false);
		}
		else
		{
			if (TraceHitboxComponent) TraceHitboxComponent->DeactivateTrace();
			AttackEnd();
		}
	}
}

void ABossEnemy::PerformHeavyAttack()
{
	if (IsDead() || IsStunned() || HeavyAttackMontages.Num() == 0)
	{
		AttackEnd();
		return;
	}

	int32 Index = FMath::RandRange(0, HeavyAttackMontages.Num() - 1);
	UAnimMontage* MontageToPlay = HeavyAttackMontages[Index];

	if (MontageToPlay)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Performing Heavy Attack: %s"), *GetName(), *MontageToPlay->GetName());
		
		if (CombatTarget)
		{
			FVector Direction = (CombatTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
			Direction.Z = 0.f;
			SetActorRotation(Direction.Rotation());
		}

		// [Fix] 强制开启攻击判定
		if (TraceHitboxComponent)
		{
			TraceHitboxComponent->SetHeavyAttack(true); // 标记重攻击
			TraceHitboxComponent->ActivateTrace();
		}

		float Duration = PlayAnimMontage(MontageToPlay);
		bIsInvulnerable = false;

		if (Duration > 0.f)
		{
			GetWorldTimerManager().SetTimer(AttackEndTimer, [this]()
			{
				if (TraceHitboxComponent)
				{
					TraceHitboxComponent->DeactivateTrace();
					TraceHitboxComponent->SetHeavyAttack(false); // 重置标记
				}
				this->AttackEnd();
			}, Duration, false);
		}
		else
		{
			if (TraceHitboxComponent) TraceHitboxComponent->DeactivateTrace();
			AttackEnd();
		}
	}
}

ABossEnemy::ABossEnemy()
{
	// Boss 通常体型较大，可以在这里调整胶囊体或移动速度
	// 增加血量
	if (HealthComponent)
	{
		// 注意：这里只是默认值，最好在蓝图中设置
		// HealthComponent->MaxHealth = 5000.0f; 
	}

	// 提高移动速度
	ChasingSpeed = 600.0f;
	PatrollingSpeed = 300.0f;
	
	// 增加攻击范围
	AttackRadius = 250.0f;
}

void ABossEnemy::BeginPlay()
{
	Super::BeginPlay();

	// 彻底禁用/销毁父类的通用闪避组件，防止它“抢戏”
	// 我们现在使用 Boss 专用的 Perfect Dodge 逻辑
	if (DodgeComponent)
	{
		DodgeComponent->DestroyComponent();
		DodgeComponent = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("[%s] Inherited DodgeComponent Destroyed to prevent conflict."), *GetName());
	}
	
	// 创建 Boss 血条 UI
	if (BossHealthBarClass)
	{
		BossHealthBarWidget = CreateWidget<UBossHealthBar>(GetWorld(), BossHealthBarClass);
		if (BossHealthBarWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Health Bar Widget Created Successfully!"), *GetName());
			// 初始化 Widget，传入 HealthComponent
			BossHealthBarWidget->InitializeWidget(HealthComponent);
			
			BossHealthBarWidget->AddToViewport();
			BossHealthBarWidget->SetVisibility(ESlateVisibility::Hidden);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] Failed to create Health Bar Widget!"), *GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] BossHealthBarClass is NONE! Please set it in Blueprint Class Defaults."), *GetName());
	}
}

void ABossEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Boss 特有的 Tick 逻辑：一旦激活就一直追击攻击，直到战斗结束
	if (IsDead() || bIsInvulnerable) return;

	// 如果有战斗目标，Boss就一直追击
	if (CombatTarget && IsValid(CombatTarget))
	{
		const float DistanceToTarget = FVector::Dist(GetActorLocation(), CombatTarget->GetActorLocation());

		// Boss一旦激活就持续追击，不判断距离
		// 如果不在攻击状态，就一直追击目标
		if (EnemyState != EEnemyState::EES_Attacking && EnemyState != EEnemyState::EES_Engaged)
		{
			// 确保处于追击状态
			if (EnemyState != EEnemyState::EES_Chasing)
			{
				EnemyState = EEnemyState::EES_Chasing;
				GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
			}

			// 使用AI控制器移动到目标
			if (AAIController* AIController = Cast<AAIController>(GetController()))
			{
				// 只在没有移动时才重新请求移动，避免每帧重置路径
				if (AIController->GetMoveStatus() == EPathFollowingStatus::Idle)
				{
					FAIMoveRequest MoveRequest;
					MoveRequest.SetGoalActor(CombatTarget);
					// 接受半径设为攻击范围，这样到达攻击范围时会自动停下
					MoveRequest.SetAcceptanceRadius(AttackRadius - 50.0f);
					AIController->MoveTo(MoveRequest);
				}

				// 面向目标
				AIController->SetFocus(CombatTarget);
			}

			// 如果进入攻击范围，开始攻击
			if (DistanceToTarget <= AttackRadius)
			{
				// 停止移动，开始攻击
				if (AAIController* AIController = Cast<AAIController>(GetController()))
				{
					AIController->StopMovement();
				}
				StartAttackTimer();
			}
		}
	}
}

void ABossEnemy::ActivateBoss(AActor* Target)
{
	if (IsDead()) return;

	UE_LOG(LogTemp, Warning, TEXT("[%s] Boss Activated! Target: %s"), *GetName(), *Target->GetName());

	// 1. 显示血条
	SetBossHealthVisibility(true);

	// 2. 设置仇恨目标
	CombatTarget = Target;

	// 3. 通知 AI 控制器
	if (AAIController* AI = Cast<AAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
		{
			BB->SetValueAsObject(TEXT("TargetActor"), Target);
			BB->SetValueAsBool(TEXT("HasAggro"), true);
		}
	}

	// 4. 播放咆哮/登场动画
	if (AggroMontage)
	{
		PlayAnimMontage(AggroMontage);
	}
	else
	{
		// 如果没有动画，直接开始追击
		ChaseTarget();
	}
}

void ABossEnemy::ReceiveDamage(float Damage, AActor* DamageInstigator)
{
	// 1. 如果处于无敌状态，忽略伤害
	if (bIsInvulnerable) return;

	// [Fix] 优先检查阶段转换：如果血量到了临界点，立即锁血/转阶段，防止被这一击打死
	if (!bHasEnteredPhase2 && HealthComponent)
	{
		float HealthAfterDamage = HealthComponent->GetCurrentHealth() - Damage;
		float HealthPercentAfter = HealthAfterDamage / HealthComponent->GetMaxHealth();
		
		if (HealthPercentAfter <= Phase2Threshold)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Critical Hit! HP would drop to %.2f. Triggering Phase Transition instead of dying."), *GetName(), HealthPercentAfter);
			EnterPhase2();
			return; // 立即返回，不执行后续伤害逻辑
		}
	}

	// [Debug Log] 
	// 注意：IsAttacking() 在基类里指“等待计时器”；IsEngaged() 才指“正在出招动画”
	UE_LOG(LogTemp, Warning, TEXT("[BossEnemy] ReceiveDamage Called! Checking Dodge: IsEngaged(Swinging): %s, IsStunned: %s"), 
		IsEngaged() ? TEXT("True") : TEXT("False"), 
		IsStunned() ? TEXT("True") : TEXT("False"));

	// 2. 尝试闪避 (现在允许在“等待攻击”时闪避，但不允许在“出招中”闪避)
	if (!IsEngaged() && !IsStunned() && !bIsInvulnerable)
	{
		float Roll = FMath::RandRange(0.0f, 1.0f);
		// 恢复 40% 闪避率
		if (Roll < 0.4f) 
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Boss Perfect Dodge! (Roll: %.2f < 0.4)"), *GetName(), Roll);
			
			if (DamageInstigator) CombatTarget = DamageInstigator;

			PerformDodge();
			return;
		}
	}

	// 3. 正常承受伤害
	Super::ReceiveDamage(Damage, DamageInstigator);
}

void ABossEnemy::CheckPhaseTransition()
{
	if (bHasEnteredPhase2 || !HealthComponent) return;

	float HealthPercent = HealthComponent->GetCurrentHealth() / HealthComponent->GetMaxHealth();
	
	if (HealthPercent <= Phase2Threshold)
	{
		EnterPhase2();
	}
}

void ABossEnemy::EnterPhase2()
{
	bHasEnteredPhase2 = true;
	CurrentPhase = EBossPhase::Phase2;
	
	UE_LOG(LogTemp, Warning, TEXT("[%s] Entering Phase 2!"), *GetName());

	// 1. 开启无敌 (防止转阶段被打断)
	bIsInvulnerable = true;

	// 2. 停止当前动作
	StopAnimMontage();
	if (GetController())
	{
		GetController()->StopMovement();
	}

	// 3. 播放转阶段动画
	float Duration = 2.0f;
	if (Phase2TransitionMontage)
	{
		Duration = PlayAnimMontage(Phase2TransitionMontage);
	}

	// 4. 强化属性 (例如：增加攻击力、速度)
	ChasingSpeed *= 1.2f;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;

	// 5. 恢复无敌状态 (在动画结束后)
	FTimerHandle PhaseTimer;
	GetWorldTimerManager().SetTimer(PhaseTimer, [this]()
	{
		bIsInvulnerable = false;
		// 可以在这里触发二阶段的第一个技能，比如召唤哮天犬
		SummonDog();
	}, Duration, false);
}

void ABossEnemy::PerformDodge()
{
	if (DodgeMontages.Num() == 0 || IsDead()) return;

	// 随机选择一个闪避动作
	int32 Index = FMath::RandRange(0, DodgeMontages.Num() - 1);
	UAnimMontage* DodgeMontage = DodgeMontages[Index];

	if (DodgeMontage)
	{
		UE_LOG(LogTemp, Log, TEXT("[%s] Boss Dodging! Montage: %s"), *GetName(), *DodgeMontage->GetName());

		// 停止当前移动，防止滑步，并允许蒙太奇RootMotion完全控制（如果有）
		if (AAIController* AI = Cast<AAIController>(GetController()))
		{
			AI->StopMovement();
		}
		
		float Duration = PlayAnimMontage(DodgeMontage);
		
		if (Duration > 0.0f)
		{
			UE_LOG(LogTemp, Warning, TEXT(">>> Dodge Montage Started! Duration: %.2f sec. If you don't see it, CHECK ANIM GRAPH SLOT NODE! <<<"), Duration);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT(">>> Dodge Montage Failed to Play! Duration = 0. Check if Montage is valid. <<<"));
		}

		// 闪避期间无敌
		bIsInvulnerable = true;
		
		// 如果返回0，为了防止卡死无敌状态，给一个最小持续时间
		if (Duration <= 0.0f) Duration = 0.5f;

		GetWorldTimerManager().SetTimer(DodgeTimer, [this]()
		{
			bIsInvulnerable = false;
			UE_LOG(LogTemp, Log, TEXT("[%s] Dodge Finished. Invulnerability OFF."), *GetName());
		}, Duration, false);
	}
}

void ABossEnemy::SummonDog()
{
	if (!DogClass) return;

	UE_LOG(LogTemp, Warning, TEXT("[%s] Summoning Dog!"), *GetName());

	// 播放召唤动画
	if (SummonDogMontage)
	{
		PlayAnimMontage(SummonDogMontage);
	}

	// 在前方生成哮天犬
	FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 300.0f;
	FRotator SpawnRotation = GetActorRotation();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	
	GetWorld()->SpawnActor<AActor>(DogClass, SpawnLocation, SpawnRotation, SpawnParams);
}

void ABossEnemy::SetBossHealthVisibility(bool bVisible)
{
	if (BossHealthBarWidget)
	{
		BossHealthBarWidget->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void ABossEnemy::Attack()
{
	if (IsDead() || IsStunned()) return;

	// 停止移动
	if (GetController()) GetController()->StopMovement();

	// 设置状态
	EnemyState = EEnemyState::EES_Engaged;

	UE_LOG(LogTemp, Warning, TEXT("[%s] Boss Attack Decision..."), *GetName());

	// 简单的随机逻辑 (后续可以用行为树替代)
	// 70% 轻攻击， 30% 重攻击
	if (FMath::RandRange(0.0f, 1.0f) < 0.7f)
	{
		PerformLightAttack();
	}
	else
	{
		PerformHeavyAttack();
	}
}


