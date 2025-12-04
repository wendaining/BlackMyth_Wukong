#include "EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/HealthComponent.h"
#include "Components/CombatComponent.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建组件
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	// 设置默认 AI 控制器
	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 敌人通常不需要摄像机碰撞检测
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	
	// 调整胶囊体大小
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// 配置角色移动
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	// CurrentHealth = MaxHealth; // Managed by HealthComponent
	
	EnemyController = Cast<AAIController>(GetController());
	
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &AEnemyBase::HandleDeath);
	}

	// 初始化状态
	StartPatrolling();
	
	// 隐藏血条 (如果实现了 UI)
	HideHealthBar();
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;

	// 逻辑已移至行为树
}

void AEnemyBase::ReceiveDamage(float Damage, AActor* DamageInstigator)
{
	if (IsDead()) return;

	if (HealthComponent)
	{
		HealthComponent->TakeDamage(Damage, DamageInstigator);
	}

	// 显示血条
	ShowHealthBar();

	// 仇恨机制：如果被攻击，立即将攻击者设为目标
	if (DamageInstigator)
	{
		CombatTarget = DamageInstigator;
		ClearPatrolTimer();
		ClearAttackTimer();
		
		// 如果在攻击范围内，尝试反击
		if (IsInsideAttackRadius() && !IsAttacking())
		{
			StartAttackTimer();
		}
		// 否则追击
		else if (IsOutsideAttackRadius())
		{
			ChaseTarget();
		}
	}

	// 播放受击动画
	if (HitReactMontage)
	{
		// 停止当前的攻击
		StopAnimMontage(AttackMontage);
		PlayAnimMontage(HitReactMontage);
	}

	// Death is handled by HealthComponent delegate
}

void AEnemyBase::Die()
{
	if (IsDead()) return;
	
	EnemyState = EEnemyState::EES_Dead;

	// 播放死亡动画
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}

	// 隐藏血条
	HideHealthBar();

	// 清除计时器
	ClearAttackTimer();
	ClearPatrolTimer();

	// 禁用碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 禁用移动
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// 停止 AI 逻辑
	if (EnemyController)
	{
		EnemyController->StopMovement();
	}

	// 设置销毁定时器（例如 5 秒后消失）
	SetLifeSpan(5.0f);
}

void AEnemyBase::Attack()
{
	if (CombatTarget == nullptr || IsDead()) return;
	
	EnemyState = EEnemyState::EES_Engaged;
	
	if (AttackMontage)
	{
		PlayAnimMontage(AttackMontage);
	}
}

void AEnemyBase::AttackEnd()
{
	EnemyState = EEnemyState::EES_NoState;
	CheckCombatTarget();
}

void AEnemyBase::CheckCombatTarget()
{
	// 逻辑移至行为树
}

void AEnemyBase::CheckPatrolTarget()
{
	// 逻辑移至行为树
}

void AEnemyBase::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

void AEnemyBase::HideHealthBar()
{
	// TODO: 实现 UI 隐藏逻辑
}

void AEnemyBase::ShowHealthBar()
{
	// TODO: 实现 UI 显示逻辑
}

void AEnemyBase::StartPatrolling()
{
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	MoveToTarget(PatrolTarget);
}

void AEnemyBase::ChaseTarget()
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

void AEnemyBase::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return;
	
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(15.0f); // 稍微小一点的接受半径
	EnemyController->MoveTo(MoveRequest);
}

AActor* AEnemyBase::ChoosePatrolTarget()
{
	TArray<AActor*> ValidTargets;
	for (AActor* Target : PatrolTargets)
	{
		if (Target != PatrolTarget)
		{
			ValidTargets.AddUnique(Target);
		}
	}

	const int32 NumPatrolTargets = ValidTargets.Num();
	if (NumPatrolTargets > 0)
	{
		const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets - 1);
		return ValidTargets[TargetSelection];
	}
	return nullptr;
}

void AEnemyBase::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemyBase::Attack, AttackTime);
}

void AEnemyBase::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

void AEnemyBase::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

bool AEnemyBase::IsOutsideCombatRadius()
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemyBase::IsOutsideAttackRadius()
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemyBase::IsInsideAttackRadius()
{
	return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemyBase::IsChasing()
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemyBase::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemyBase::IsDead()
{
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemyBase::IsEngaged()
{
	return EnemyState == EEnemyState::EES_Engaged;
}

float AEnemyBase::GetCurrentHealth() const
{
	return HealthComponent ? HealthComponent->GetCurrentHealth() : 0.f;
}

float AEnemyBase::GetMaxHealth() const
{
	return HealthComponent ? HealthComponent->GetMaxHealth() : 0.f;
}

void AEnemyBase::HandleDeath(AActor* Killer)
{
	Die();
}

bool AEnemyBase::InTargetRange(AActor* Target, double Radius)
{
	if (Target == nullptr) return false;
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;
}
