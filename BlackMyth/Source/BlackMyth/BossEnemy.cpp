#include "BossEnemy.h"
#include "BossHealthBar.h"
#include "Kismet/GameplayStatics.h"
#include "Components/HealthComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

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

	// 创建 Boss 血条 UI
	if (BossHealthBarClass)
	{
		BossHealthBarWidget = CreateWidget<UBossHealthBar>(GetWorld(), BossHealthBarClass);
		if (BossHealthBarWidget)
		{
			// 初始化 Widget，传入 HealthComponent
			BossHealthBarWidget->InitializeWidget(HealthComponent);
			
			BossHealthBarWidget->AddToViewport();
			BossHealthBarWidget->SetVisibility(ESlateVisibility::Hidden);
		}
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
	// 如果处于无敌状态，忽略伤害
	if (bIsInvulnerable) return;

	// 调用父类处理伤害 (扣血、受击动画等)
	// 注意：Boss 通常有霸体 (Super Armor)，可能不需要每次都播放受击动画
	// 这里我们可以重写父类的 PlayHitReactMontage 来控制霸体逻辑
	Super::ReceiveDamage(Damage, DamageInstigator);

	// 检查阶段转换
	CheckPhaseTransition();

	// 闪避逻辑：有一定概率在受击后闪避 (仅在非攻击状态下)
	if (!IsAttacking() && !IsStunned() && !bIsInvulnerable)
	{
		if (FMath::RandRange(0.0f, 1.0f) < 0.3f) // 30% 概率闪避
		{
			PerformDodge();
		}
	}
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
		UE_LOG(LogTemp, Log, TEXT("[%s] Boss Dodging!"), *GetName());
		
		PlayAnimMontage(DodgeMontage);
		
		// 闪避期间无敌
		bIsInvulnerable = true;
		
		float Duration = DodgeMontage->GetPlayLength();
		GetWorldTimerManager().SetTimer(DodgeTimer, [this]()
		{
			bIsInvulnerable = false;
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
