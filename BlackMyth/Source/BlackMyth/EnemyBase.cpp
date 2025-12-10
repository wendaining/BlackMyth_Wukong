#include "EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/HealthComponent.h"
#include "Components/CombatComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/EnemyHealthBarWidget.h"

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

	// ========== 新增：创建血条组件 ==========
	HealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidgetComponent->SetupAttachment(GetRootComponent());
	HealthBarWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));  // 头顶上方
	HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);  // 屏幕空间，始终面向摄像机
	HealthBarWidgetComponent->SetDrawSize(FVector2D(150.0f, 20.0f));
	HealthBarWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthBarWidgetComponent->SetVisibility(false);  // 默认隐藏
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
	bHasAggroed = false;

	// ========== 新增：初始化血条 ==========
	if (HealthBarWidgetComponent && HealthBarWidgetClass)
	{
		HealthBarWidgetComponent->SetWidgetClass(HealthBarWidgetClass);
		HealthBarWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, HealthBarHeightOffset));

		// 获取 Widget 实例并初始化
		HealthBarWidget = Cast<UEnemyHealthBarWidget>(HealthBarWidgetComponent->GetWidget());
		if (HealthBarWidget && HealthComponent)
		{
			HealthBarWidget->InitializeHealthBar(HealthComponent);
		}
	}

	// 如果设置为始终显示，则显示血条
	if (bAlwaysShowHealthBar)
	{
		ShowHealthBar();
	}
	else {
		// 隐藏血条 (如果实现了 UI)
		HideHealthBar();
	}
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;

	// 如果处于追击状态，且有目标
	if (EnemyState == EEnemyState::EES_Chasing && CombatTarget)
	{
		// 计算与目标的距离
		double DistanceToTarget = (CombatTarget->GetActorLocation() - GetActorLocation()).Size();

		// 如果距离大于攻击范围，继续移动
		if (DistanceToTarget > AttackRadius)
		{
			MoveToTarget(CombatTarget);
		}
		// 如果进入攻击范围，且没有在攻击，尝试攻击
		else if (DistanceToTarget <= AttackRadius && !IsAttacking())
		{
			// 停止移动
			if (EnemyController) EnemyController->StopMovement();
			
			// 开始攻击 (这里简单直接调用，也可以用定时器)
			StartAttackTimer();
		}
	}
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
		
		// 计算并播放定向受击动画
		PlayHitReactMontage(DamageInstigator->GetActorLocation());

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
	// 如果没有攻击者（例如环境伤害），只播放正面受击
	else 
	{
		PlayHitReactMontage(GetActorLocation() + GetActorForwardVector() * 100.f);
	}

	// Death is handled by HealthComponent delegate
}

void AEnemyBase::PlayHitReactMontage(const FVector& ImpactPoint)
{
	// 如果已经死亡，不播放
	if (IsDead()) return;

	// 停止当前的攻击动画
	StopAnimMontage(AttackMontage);

	// 计算攻击方向向量 (攻击者位置 - 自身位置)
	const FVector ImpactLowered(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();

	// 获取角色正前方向量
	const FVector Forward = GetActorForwardVector();
	
	// 使用点乘判断前后 (Cosθ)
	// Forward · ToHit > 0 表示在前方，< 0 表示在后方
	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	
	// 使用叉乘判断左右
	// (Forward x ToHit).Z > 0 表示在右侧，< 0 表示在左侧
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);

	UAnimMontage* MontageToPlay = nullptr;

	if (CosTheta >= 0.5f) // 夹角在 60度以内，视为正面
	{
		MontageToPlay = HitReactMontage_Front;
	}
	else if (CosTheta <= -0.5f) // 夹角在 120度以外，视为背面
	{
		MontageToPlay = HitReactMontage_Back;
	}
	else // 侧面
	{
		if (CrossProduct.Z > 0)
		{
			MontageToPlay = HitReactMontage_Right;
		}
		else
		{
			MontageToPlay = HitReactMontage_Left;
		}
	}

	// 播放选中的蒙太奇
	if (MontageToPlay)
	{
		PlayAnimMontage(MontageToPlay);
	}
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

void AEnemyBase::ShowHealthBar()
{
	if (HealthBarWidgetComponent)
	{
		HealthBarWidgetComponent->SetVisibility(true);
	}
}

void AEnemyBase::HideHealthBar()
{
	if (HealthBarWidgetComponent)
	{
		HealthBarWidgetComponent->SetVisibility(false);
	}
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

void AEnemyBase::OnTargetSensed(AActor* Target)
{
	// 如果已经发现过，或者已经死了，就不再处理
	if (bHasAggroed || IsDead()) return;

	bHasAggroed = true;
	CombatTarget = Target;

	// 停止移动
	if (EnemyController)
	{
		EnemyController->StopMovement();
	}

	// 播放咆哮动画
	float Duration = 0.0f;
	if (AggroMontage)
	{
		Duration = PlayAnimMontage(AggroMontage);
		EnemyState = EEnemyState::EES_Engaged; // 设为交战状态，防止其他逻辑干扰
	}

	// 如果没有动画，Duration 为 0，直接开始追击
	// 如果有动画，等待动画播放完再追击
	if (Duration > 0.0f)
	{
		GetWorldTimerManager().SetTimer(AggroTimer, this, &AEnemyBase::StartChasingAfterAggro, Duration, false);
	}
	else
	{
		StartChasingAfterAggro();
	}
}

void AEnemyBase::StartChasingAfterAggro()
{
	// 切换到追击状态
	EnemyState = EEnemyState::EES_Chasing;
	
	// 设置更快的速度
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	
	// 确保朝向目标
	if (CombatTarget)
	{
		ChaseTarget();
	}
}
