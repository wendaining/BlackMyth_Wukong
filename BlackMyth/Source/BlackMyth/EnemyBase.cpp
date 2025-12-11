#include "EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/HealthComponent.h"
#include "Components/CombatComponent.h"
#include "Combat/TraceHitboxComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/EnemyHealthBarWidget.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimInstance.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建组件
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	TraceHitboxComponent = CreateDefaultSubobject<UTraceHitboxComponent>(TEXT("TraceHitboxComponent"));
	// TraceHitboxComponent 是 ActorComponent，不需要 SetupAttachment
	// TraceHitboxComponent->SetupAttachment(GetRootComponent());

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

	// 默认攻击范围 (稍微加大一点，避免贴得太近)
	AttackRadius = 200.0f;

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

	// 调试日志：检查蒙太奇是否正确加载
	if (AttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("AEnemyBase::BeginPlay - AttackMontage is SET: %s"), *AttackMontage->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AEnemyBase::BeginPlay - AttackMontage is NULL! Please check Blueprint assignment."));
	}

	if (AggroMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("AEnemyBase::BeginPlay - AggroMontage is SET: %s"), *AggroMontage->GetName());
	}

	// 强制应用巡逻速度，确保蓝图配置生效
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;

	// 统计场景中的敌人数量
	TArray<AActor*> AllEnemies;
	UGameplayStatics::GetAllActorsOfClass(this, AEnemyBase::StaticClass(), AllEnemies);
	UE_LOG(LogTemp, Warning, TEXT("AEnemyBase::BeginPlay - Total Enemies: %d. I am: %s. AttackRadius: %f"), AllEnemies.Num(), *GetName(), AttackRadius);

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

	// 尝试自动配置 TraceHitbox 的 Socket
	if (TraceHitboxComponent && GetMesh())
	{
		// 尝试常见的武器骨骼名称
		const TArray<FName> PossibleStartSockets = { FName("weapon_r"), FName("hand_r"), FName("fx_trail_01") };
		const TArray<FName> PossibleEndSockets = { FName("weapon_t"), FName("weapon_tip"), FName("fx_trail_02"), FName("middle_01_r") };

		for (const FName& Name : PossibleStartSockets)
		{
			if (GetMesh()->DoesSocketExist(Name) || GetMesh()->GetBoneIndex(Name) != INDEX_NONE)
			{
				TraceHitboxComponent->SetStartSocket(Name);
				UE_LOG(LogTemp, Warning, TEXT("AEnemyBase::BeginPlay - Auto-configured StartSocket: %s"), *Name.ToString());
				break;
			}
		}

		for (const FName& Name : PossibleEndSockets)
		{
			if (GetMesh()->DoesSocketExist(Name) || GetMesh()->GetBoneIndex(Name) != INDEX_NONE)
			{
				TraceHitboxComponent->SetEndSocket(Name);
				UE_LOG(LogTemp, Warning, TEXT("AEnemyBase::BeginPlay - Auto-configured EndSocket: %s"), *Name.ToString());
				break;
			}
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

	if (CombatTarget)
	{
		const double DistanceToTarget = (CombatTarget->GetActorLocation() - GetActorLocation()).Size();

		// 始终面向目标 (当不移动时)
		// 解决“乱转”问题：当敌人停止移动准备攻击时，平滑旋转朝向目标
		if (!IsChasing() && !GetCharacterMovement()->IsFalling())
		{
			FVector Direction = CombatTarget->GetActorLocation() - GetActorLocation();
			Direction.Z = 0.0f; // 只在水平面旋转
			if (!Direction.IsNearlyZero())
			{
				FRotator TargetRot = Direction.Rotation();
				FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 5.0f);
				SetActorRotation(NewRot);
			}
		}

		// 如果处于追击状态
		if (EnemyState == EEnemyState::EES_Chasing)
		{
			// 如果距离大于攻击范围，继续移动
			if (DistanceToTarget > AttackRadius)
			{
				// 优化：只有在没有移动时才请求移动，避免每帧调用 MoveTo 重置路径
				if (EnemyController && EnemyController->GetMoveStatus() == EPathFollowingStatus::Idle)
				{
					MoveToTarget(CombatTarget);
				}
			}
			// 如果进入攻击范围，准备攻击
			else
			{
				// 停止移动
				if (EnemyController) EnemyController->StopMovement();
				
				// 开始攻击计时
				StartAttackTimer();
			}
		}
		// 如果处于等待攻击状态 (EES_Attacking)
		else if (EnemyState == EEnemyState::EES_Attacking)
		{
			// 如果在等待攻击时目标跑远了，取消攻击并重新追击
			// 增加 50.0f 的缓冲距离 (Hysteresis)，防止在边缘反复切换状态
			if (DistanceToTarget > (AttackRadius + 50.0f))
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] AEnemyBase::Tick - Target too far (%.2f > %.2f), Canceling Attack"), *GetName(), DistanceToTarget, AttackRadius + 50.0f);
				ClearAttackTimer();
				ChaseTarget();
			}
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
		UE_LOG(LogTemp, Warning, TEXT("[%s] PlayHitReactMontage - Playing: %s"), *GetName(), *MontageToPlay->GetName());
		PlayAnimMontage(MontageToPlay);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] PlayHitReactMontage - No Montage found for this direction!"), *GetName());
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
	
	UE_LOG(LogTemp, Warning, TEXT("[%s] AEnemyBase::Attack - Attacking Target"), *GetName());
	EnemyState = EEnemyState::EES_Engaged;
	
	// 强制打印 AttackMontage 的状态
	if (AttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] AEnemyBase::Attack - Playing Montage: %s"), *GetName(), *AttackMontage->GetName());
		
		// 开启攻击判定 (保底机制：如果蒙太奇里没有加 Notify，这里强制开启)
		if (TraceHitboxComponent)
		{
			TraceHitboxComponent->ActivateTrace();
		}

		const float Duration = PlayAnimMontage(AttackMontage);
		if (Duration <= 0.0f)
		{
			UE_LOG(LogTemp, Error, TEXT("[%s] AEnemyBase::Attack - Montage failed to play! Duration=0. Forcing AttackEnd."), *GetName());
			AttackEnd();
		}
		else
		{
			// 设置保底计时器：如果动画蓝图没有发送 AttackEnd 通知，我们就在动画播放完毕后强制结束攻击
			GetWorldTimerManager().SetTimer(AttackEndTimer, this, &AEnemyBase::AttackEnd, Duration, false);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] AEnemyBase::Attack - No AttackMontage assigned! (Ptr is NULL)"), *GetName());
		AttackEnd();
	}
}

void AEnemyBase::AttackEnd()
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] AEnemyBase::AttackEnd - Attack Finished"), *GetName());
	
	// 关闭攻击判定
	if (TraceHitboxComponent)
	{
		TraceHitboxComponent->DeactivateTrace();
	}

	// 清除保底计时器（如果是通过 AnimNotify 调用的，就不需要计时器了）
	GetWorldTimerManager().ClearTimer(AttackEndTimer);

	// 修复：攻击结束后，先将状态设为 Chasing，这样如果 CheckCombatTarget 决定不攻击，
	// Tick 函数也能接管移动逻辑。
	EnemyState = EEnemyState::EES_Chasing;
	
	CheckCombatTarget();
}

void AEnemyBase::CheckCombatTarget()
{
	if (IsDead()) return;

	if (CombatTarget)
	{
		// 如果有目标，且不在攻击范围内，继续追击
		if (!IsInsideAttackRadius())
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] AEnemyBase::CheckCombatTarget - Target out of range, Chasing"), *GetName());
			// 清除攻击计时器，防止重复攻击
			ClearAttackTimer();
			
			// 重新开始追击
			ChaseTarget();
		}
		// 如果还在攻击范围内，且没有在攻击，准备下一次攻击
		else if (!IsAttacking())
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] AEnemyBase::CheckCombatTarget - Target in range, Preparing Attack"), *GetName());
			// 确保没有移动
			if (EnemyController) EnemyController->StopMovement();
			
			StartAttackTimer();
		}
	}
	else
	{
		// 丢失目标，回到巡逻
		ClearAttackTimer();
		StartPatrolling();
	}
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
	
	// 停止注视目标
	if (EnemyController)
	{
		EnemyController->ClearFocus(EAIFocusPriority::Gameplay);
	}
	
	MoveToTarget(PatrolTarget);
}

void AEnemyBase::ChaseTarget()
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	
	// 强制停止当前移动，确保新的 MoveTo 请求能生效
	if (EnemyController)
	{
		EnemyController->StopMovement();
		// 锁定注视目标，防止乱转
		EnemyController->SetFocus(CombatTarget);
	}
	
	MoveToTarget(CombatTarget);
}

void AEnemyBase::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return;
	
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	
	// 动态调整接受半径：目标是进入攻击范围，而不是贴脸
	// 留出 50.0f 的余量，确保能触发 IsInsideAttackRadius，同时避免挤在一起导致原地打转
	const float AcceptanceRadius = FMath::Max((float)AttackRadius - 50.0f, 50.0f);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius); 
	
	FNavPathSharedPtr NavPath;
	EnemyController->MoveTo(MoveRequest, &NavPath);
	
	// 简单的调试日志，检查移动请求是否发出
	// UE_LOG(LogTemp, Log, TEXT("MoveToTarget: %s"), *Target->GetName());
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
	UE_LOG(LogTemp, Warning, TEXT("[%s] AEnemyBase::StartAttackTimer - Next attack in %f seconds"), *GetName(), AttackTime);
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

	UE_LOG(LogTemp, Warning, TEXT("AEnemyBase::OnTargetSensed - Target Sensed: %s"), *Target->GetName());

	// 停止移动
	if (EnemyController)
	{
		EnemyController->StopMovement();
	}

	// 播放咆哮动画
	float Duration = 0.0f;
	if (AggroMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("AEnemyBase::OnTargetSensed - Playing AggroMontage: %s"), *AggroMontage->GetName());
		Duration = PlayAnimMontage(AggroMontage);
		EnemyState = EEnemyState::EES_Engaged; // 设为交战状态，防止其他逻辑干扰
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AEnemyBase::OnTargetSensed - No AggroMontage assigned."));
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
