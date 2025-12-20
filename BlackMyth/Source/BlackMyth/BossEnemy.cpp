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

			// [Fix] 二阶段远程突袭逻辑
			// 如果处于二阶段且距离较远，即便没进入攻击范围，也定期尝试“远程出招”（召唤狗）
			if (CurrentPhase == EBossPhase::Phase2 && DistanceToTarget > AttackRadius + 150.0f)
			{
				RangedAttackCheckTimer += DeltaTime;

				if (RangedAttackCheckTimer >= 3.5f) // 每3.5秒判断一次是否放狗
				{
					RangedAttackCheckTimer = 0.0f;
					UE_LOG(LogTemp, Warning, TEXT("[%s] Tactical Ranged Attack Decision (Dist: %.1f)"), *GetName(), DistanceToTarget);
					Attack();
					return;
				}
			}
			else
			{
				// 如果进入近战范围或不是二阶段，重置计时器
				RangedAttackCheckTimer = 0.0f;
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
	// 1. 如果处于真正的系统级无敌状态（如转阶段过程中），忽略伤害
	if (bIsInvulnerable) return;
	
	// [New] 为防止闪避过程中产生硬直冲突，我们允许受击逻辑在这里继续判定

	// [锁血保护] 优先检查阶段转换：如果血量到了临界点，立即锁血/转阶段，保证二阶段能顺利开启
	if (!bHasEnteredPhase2 && HealthComponent)
	{
		float HealthAfterDamage = HealthComponent->GetCurrentHealth() - Damage;
		if (HealthAfterDamage / HealthComponent->GetMaxHealth() <= Phase2Threshold)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] Triggering Phase Transition Lock."), *GetName());
			EnterPhase2();
			return; 
		}
	}

	// 2. [战斗大师逻辑] 尝试闪避
	// 允许在任何非眩晕状态下闪避（包括出招过程中，即“强制取消攻击并闪避”）
	if (!IsStunned() && !bIsInvulnerable)
	{
		float Roll = FMath::RandRange(0.0f, 1.0f);
		// 基础闪避率 40%
		if (Roll < 0.4f) 
		{
			UE_LOG(LogTemp, Log, TEXT("[%s] Masterful Evasion (Dodge Cancel)!"), *GetName());
			
			if (DamageInstigator) CombatTarget = DamageInstigator;

			// 如果正在出招，立即停止当前攻击蒙太奇
			if (IsEngaged())
			{
				StopAnimMontage(); // 停止当前所有蒙太奇（包括攻击）
				if (TraceHitboxComponent) TraceHitboxComponent->DeactivateTrace(); // 关闭判定框
				GetWorldTimerManager().ClearTimer(AttackEndTimer); // 清除攻击保底计时器
			}

			PerformDodge();
			return;
		}
	}

	// 3. [霸体/受击逻辑] 
	// 如果正在进行重攻击（HeavyAttack），则开启“霸体”：扣血但不中断动作
	if (IsEngaged() && GetWorldTimerManager().IsTimerActive(AttackEndTimer))
	{
		// 简单起见，我们假设当前正在播放的是重攻击（可以通过增加一个状态变量更严谨，但目前够用）
		// 我们只扣血，不调用 Super::ReceiveDamage (因为父类会执行中断逻辑)
		if (HealthComponent) HealthComponent->TakeDamage(Damage, DamageInstigator);
		return;
	}

	// 4. 正常承受伤害（会触发中断动画）
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

		// 闪避期间不再硬性设置无敌，交给 ReceiveDamage 进行动作判断
		// bIsInvulnerable = true;
		
		// 如果返回0，为了防止卡死无敌状态，给一个最小持续时间
		if (Duration <= 0.0f) Duration = 0.5f;

		GetWorldTimerManager().SetTimer(DodgeTimer, [this]()
		{
			// [Fix] 闪避结束后，重新进入追击状态，并清除由于闪避导致的系统无敌（如果有）
			bIsInvulnerable = false;
			
			if (!IsDead() && !IsStunned())
			{
				EnemyState = EEnemyState::EES_Chasing;
				UE_LOG(LogTemp, Log, TEXT("[%s] Dodge Finished. Resuming Chase."), *GetName());
			}
		}, Duration, false);
	}
}

void ABossEnemy::SummonDog()
{
	// [Debug] 如果忘记在蓝图中给 DogClass 赋值，这里会报警
	if (!DogClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] SummonDog FAILED: DogClass is NULL! Please set it in Blueprint defaults."), *GetName());
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[%s] Summoning Dog!"), *GetName());

	// 1. 设置攻击状态（防止被其它招式中断）
	EnemyState = EEnemyState::EES_Engaged;

	// 2. 播放召唤动画
	float Duration = 1.0f; // 保底时间
	if (SummonDogMontage)
	{
		Duration = PlayAnimMontage(SummonDogMontage);
	}

	// 3. 在前方生成哮天犬 (使用蓝图可调的偏移量)
	FVector SpawnLocation = GetActorLocation() + GetActorRotation().RotateVector(DogSpawnOffset);
	FRotator SpawnRotation = GetActorRotation();
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	
	GetWorld()->SpawnActor<AActor>(DogClass, SpawnLocation, SpawnRotation, SpawnParams);

	// 4. 为收招设置计时器
	GetWorldTimerManager().SetTimer(AttackEndTimer, [this]()
	{
		// 调用父类的 AttackEnd 以触发 CheckCombatTarget 并开启下一轮攻势
		this->AttackEnd();
	}, Duration, false);
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

	float Roll = FMath::RandRange(0.0f, 1.0f);

	UE_LOG(LogTemp, Log, TEXT("[%s] Boss Attack - CurrentPhase: %d, Roll: %.2f"), *GetName(), (int32)CurrentPhase, Roll);

	// ========== 二阶段专属 AI 逻辑 ==========
	if (CurrentPhase == EBossPhase::Phase2)
	{
		if (CombatTarget)
		{
			float Distance = FVector::Dist(GetActorLocation(), CombatTarget->GetActorLocation());
			
			// 二阶段核心：距离远就放狗 (提高由于距离触发的概率)
			if (Distance > 600.0f)
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] Phase 2 Distance Attack (Dist: %.1f)"), *GetName(), Distance);
				SummonDog();
				return;
			}
		}

		// 近距离下的放狗概率 (提高到 30%，让玩家感受到“狗”的存在感)
		if (Roll < 0.3f)
		{
			SummonDog();
			return;
		}
		else if (Roll < 0.65f)
		{
			PerformLightAttack();
			return;
		}
		else
		{
			PerformHeavyAttack();
			return;
		}
	}

	// ========== 一阶段逻辑 (完全禁止放狗) ==========
	// 70% 轻攻击， 30% 重攻击
	if (Roll < 0.7f)
	{
		PerformLightAttack();
	}
	else
	{
		PerformHeavyAttack();
	}
}


