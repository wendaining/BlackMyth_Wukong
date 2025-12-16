#include "EnemyBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Components/HealthComponent.h"
#include "Components/CombatComponent.h"
#include "Components/TeamComponent.h"
#include "Components/SceneStateComponent.h"
#include "Combat/TraceHitboxComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/EnemyHealthBarWidget.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/Skeleton.h"
#include "Animation/AnimInstance.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameStateBase.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建组件
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	TraceHitboxComponent = CreateDefaultSubobject<UTraceHitboxComponent>(TEXT("TraceHitboxComponent"));
	// TraceHitboxComponent 是 ActorComponent，不需要 SetupAttachment
	// TraceHitboxComponent->SetupAttachment(GetRootComponent());

	// 创建阵营组件，设置为敌人阵营
	TeamComponent = CreateDefaultSubobject<UTeamComponent>(TEXT("TeamComponent"));
	TeamComponent->SetTeam(ETeam::Enemy);

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

	// ========== 新增：创建定身"定"字 Widget 组件 ==========
	FreezeTextWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("FreezeTextWidget"));
	FreezeTextWidgetComponent->SetupAttachment(GetRootComponent());
	FreezeTextWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 180.0f));  // 头顶上方（比血条更高）
	FreezeTextWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);  // 屏幕空间，始终面向摄像机
	FreezeTextWidgetComponent->SetDrawSize(FVector2D(100.0f, 100.0f));
	FreezeTextWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FreezeTextWidgetComponent->SetVisibility(false);  // 默认隐藏

	// ========== 加载定身术默认资产 ==========
	// 默认"定"字 Widget 类（路径需要在创建蓝图后设置）
	static ConstructorHelpers::FClassFinder<UUserWidget> DefaultFreezeWidgetClass(
		TEXT("/Game/_BlackMythGame/UI/WBP_FreezeText"));
	if (DefaultFreezeWidgetClass.Succeeded())
	{
		FreezeTextWidgetClass = DefaultFreezeWidgetClass.Class;
	}

	// 默认定身音效（可选，放在 Content/_BlackMythGame/Audio/ 下）
	// static ConstructorHelpers::FObjectFinder<USoundBase> DefaultFreezeSound(
	// 	TEXT("/Game/_BlackMythGame/Audio/SFX_Freeze"));
	// if (DefaultFreezeSound.Succeeded())
	// {
	// 	FreezeSound = DefaultFreezeSound.Object;
	// }
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	// CurrentHealth = MaxHealth; // Managed by HealthComponent
	
	// 添加敌人标签，用于目标锁定系统识别
	Tags.AddUnique(FName("Enemy"));
	
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

	// ========== 新增：生成武器 ==========
	if (WeaponClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CurrentWeapon = GetWorld()->SpawnActor<AActor>(WeaponClass, GetActorTransform(), SpawnParams);
		if (CurrentWeapon)
		{
			// 附加到插槽
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, WeaponSocketName);
			UE_LOG(LogTemp, Warning, TEXT("[%s] Spawned Weapon: %s attached to %s"), *GetName(), *CurrentWeapon->GetName(), *WeaponSocketName.ToString());

			// 关键修复：让武器忽略持有者的碰撞，防止被弹飞
			// 遍历武器的所有 PrimitiveComponent (如 CollisionSphere, StaticMesh 等)
			TArray<UPrimitiveComponent*> WeaponComps;
			CurrentWeapon->GetComponents(WeaponComps);
			for (auto* Comp : WeaponComps)
			{
				if (Comp)
				{
					// 让武器组件忽略角色
					Comp->IgnoreActorWhenMoving(this, true);
					// 让角色胶囊体忽略武器组件
					GetCapsuleComponent()->IgnoreComponentWhenMoving(Comp, true);
				}
			}

			// 尝试查找武器的 Mesh 组件，用于 Hitbox 扫描
			// 优先查找 SkeletalMesh (如复杂武器)，其次查找 StaticMesh (如简单武器)
			USceneComponent* WeaponMesh = CurrentWeapon->FindComponentByClass<USkeletalMeshComponent>();
			if (!WeaponMesh)
			{
				WeaponMesh = CurrentWeapon->FindComponentByClass<UStaticMeshComponent>();
			}

			// 如果找到了武器 Mesh，更新 Hitbox 组件的引用
			if (WeaponMesh && TraceHitboxComponent)
			{
				TraceHitboxComponent->SetMeshToTrace(WeaponMesh);
				UE_LOG(LogTemp, Log, TEXT("[%s] Updated TraceHitbox to use Weapon Mesh: %s"), *GetName(), *WeaponMesh->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] Weapon spawned but no Mesh found for Hitbox!"), *GetName());
			}
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

	// 初始化韧性
	CurrentPoise = MaxPoise;
	LastHitTime = -100.0; // 确保一开始就能恢复
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsDead()) return;

	// 韧性恢复逻辑：只有当 (当前时间 - 上次受击时间) > 恢复延迟时，才开始恢复
	const double TimeSinceLastHit = GetWorld()->GetTimeSeconds() - LastHitTime;
	if (!IsStunned() && CurrentPoise < MaxPoise && TimeSinceLastHit > PoiseRecoveryDelay)
	{
		CurrentPoise = FMath::Min(CurrentPoise + PoiseRecoveryRate * DeltaTime, MaxPoise);
	}

	if (CombatTarget)
	{
		const double DistanceToTarget = (CombatTarget->GetActorLocation() - GetActorLocation()).Size();

		// 始终面向目标 (当不移动时)
		// 解决“乱转”问题：当敌人停止移动准备攻击时，平滑旋转朝向目标
		// [修复] 增加 !IsStunned() 检查，防止眩晕时还在转
		if (!IsChasing() && !GetCharacterMovement()->IsFalling() && !IsStunned())
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
				// [Fix] 移除 C++ 直接移动逻辑，交由行为树处理，防止“透视”Bug
				/*
				if (EnemyController && EnemyController->GetMoveStatus() == EPathFollowingStatus::Idle)
				{
					MoveToTarget(CombatTarget);
				}
				*/
			}
			// 如果进入攻击范围，准备攻击
			else
			{
				// [Fix] 不要在这里调用 StopMovement()！
				// 如果在这里强制停止移动，会导致行为树里的 MoveTo 任务被判定为 Failed (Aborted)。
				// 一旦 MoveTo 失败，行为树的 Combat Sequence 就会中断，导致 AI 回退到 Patrol 分支，
				// 表现为“走到面前突然转身跑远”。
				// 我们应该信任行为树会自己走到目的地停下，或者在 Attack() 真正开始时再停止。
				// if (EnemyController) EnemyController->StopMovement();
				
				// 开始攻击计时
				StartAttackTimer();
			}
		}
		// 如果处于等待攻击状态 (EES_Attacking)
		else if (EnemyState == EEnemyState::EES_Attacking)
		{
			// 如果在等待攻击时目标跑远了，取消攻击并重新追击
			// 增加 100.0f 的缓冲距离 (Hysteresis)，防止在边缘反复切换状态
			// 之前是 50.0f，可能太小了，导致稍微一动就取消攻击
			if (DistanceToTarget > (AttackRadius + 100.0f))
			{
				UE_LOG(LogTemp, Warning, TEXT("[%s] AEnemyBase::Tick - Target too far (%.2f > %.2f), Canceling Attack"), *GetName(), DistanceToTarget, AttackRadius + 100.0f);
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

	// 如果受到伤害后死亡，立即停止后续逻辑
	if (IsDead()) return;

	// 显示血条
	ShowHealthBar();

	// 仇恨机制：如果被攻击，立即将攻击者设为目标
	if (DamageInstigator)
	{
		CombatTarget = DamageInstigator;
		
		// [Fix] 同步更新黑板，确保行为树知道目标是谁
		if (EnemyController)
		{
			if (UBlackboardComponent* BB = EnemyController->GetBlackboardComponent())
			{
				BB->SetValueAsObject(TEXT("TargetActor"), CombatTarget);
			}
		}

		ClearPatrolTimer();
		ClearAttackTimer();
		
		// 播放受击音效
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
		}

		// [新增] 物理击退逻辑
		// 计算击退方向：从攻击者指向受击者
		FVector KnockbackDirection = (GetActorLocation() - DamageInstigator->GetActorLocation()).GetSafeNormal();
		KnockbackDirection.Z = 0.0f; // 保持水平，不飞天
		
		// 击退力度 (您可以调整这个数值，比如 500, 800, 1000)
		const float KnockbackStrength = 800.0f; 
		
		// 施加力 (LaunchCharacter 是 Character 类的内置函数)
		LaunchCharacter(KnockbackDirection * KnockbackStrength, true, true);

		// [新增] 韧性扣除逻辑
		CurrentPoise -= Damage; // 假设伤害值等于削韧值，也可以单独传参
		LastHitTime = GetWorld()->GetTimeSeconds(); // 记录受击时间
		
		// 调试日志：打印当前韧性
		UE_LOG(LogTemp, Warning, TEXT("[%s] Took %.1f Damage. Poise: %.1f / %.1f"), *GetName(), Damage, CurrentPoise, MaxPoise);

		if (CurrentPoise <= 0.0f)
		{
			// 记录之前的状态，用于判断是否是新触发的眩晕
			const bool bWasStunned = IsStunned();

			// 触发大硬直 (Stun)
			CurrentPoise = 0.0f; // 归零
			
			// 停止移动和攻击
			if (EnemyController) EnemyController->StopMovement();
			ClearAttackTimer();
			GetWorldTimerManager().ClearTimer(AttackEndTimer); // [Fix] 必须清除攻击结束计时器，否则它会重置状态
			
			// 强制停止当前的攻击蒙太奇 (如果正在攻击)
			if (IsAttacking() && AttackMontage)
			{
				StopAnimMontage(AttackMontage);
			}
			// 强制关闭攻击判定框 (防止眩晕时武器还有伤害)
			if (TraceHitboxComponent)
			{
				TraceHitboxComponent->DeactivateTrace();
			}

			// 切换状态
			EnemyState = EEnemyState::EES_Stunned;

			// 播放眩晕音效 (仅在初次进入眩晕时播放，避免连续攻击时太吵)
			if (StunSound && !bWasStunned)
			{
				UGameplayStatics::PlaySoundAtLocation(this, StunSound, GetActorLocation());
			}
			
			// 播放眩晕动画 (每次受击都重播，实现连续控制)
			if (StunMontage)
			{
				PlayAnimMontage(StunMontage);
				// 设置恢复计时器
				GetWorldTimerManager().SetTimer(StunTimer, this, &AEnemyBase::StunEnd, StunDuration, false);
				UE_LOG(LogTemp, Warning, TEXT("[%s] Stunned! Poise Broken."), *GetName());
			}
			else
			{
				// 如果没有眩晕动画，就只播放普通受击，并重置韧性
				CurrentPoise = MaxPoise;
				PlayHitReactMontage(DamageInstigator->GetActorLocation());
			}
		}
		else
		{
			// 韧性未破，只播放普通受击
			// [Fix] 如果已经眩晕，不要播放受击动画，否则会打断眩晕动画导致“秒醒”
			if (!IsStunned())
			{
				PlayHitReactMontage(DamageInstigator->GetActorLocation());
			}
		}

		// 如果在攻击范围内，尝试反击 (仅在未眩晕时)
		if (!IsStunned())
		{
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

	// 播放死亡音效
	if (DeathSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	}

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
	GetWorldTimerManager().ClearTimer(AggroTimer);
	GetWorldTimerManager().ClearTimer(AttackEndTimer);

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
		if (UBrainComponent* Brain = EnemyController->GetBrainComponent())
		{
			Brain->StopLogic("Dead");
		}
	}

	// 强制关闭攻击判定 (防止死后还能造成伤害)
	if (TraceHitboxComponent)
	{
		TraceHitboxComponent->DeactivateTrace();
	}

	// 冻结动画 (防止蒙太奇播放完后瞬移回 Idle)
	// 我们设置一个计时器，在蒙太奇播放完的那一刻暂停动画
	if (DeathMontage)
	{
		const float DeathDuration = DeathMontage->GetPlayLength();
		FTimerHandle DeathFreezeTimer;
		GetWorldTimerManager().SetTimer(DeathFreezeTimer, [this]()
		{
			if (GetMesh())
			{
				GetMesh()->bPauseAnims = true;
				GetMesh()->SetComponentTickEnabled(false);
			}
		}, DeathDuration - 0.1f, false); // 提前 0.1秒冻结，确保停在最后一帧
	}

	// 设置销毁定时器（例如 5 秒后消失）
	SetLifeSpan(5.0f);
}

void AEnemyBase::Attack()
{
	if (CombatTarget == nullptr || IsDead()) return;
	if (IsStunned()) return; // [Fix] 眩晕状态下禁止攻击
	
	// [Fix] 在攻击真正开始时停止移动，防止滑步
	// 此时就算行为树中断也没关系，因为攻击动作已经接管了表现
	if (EnemyController) EnemyController->StopMovement();

	UE_LOG(LogTemp, Warning, TEXT("[%s] AEnemyBase::Attack - Attacking Target"), *GetName());
	EnemyState = EEnemyState::EES_Engaged;
	
	// 强制打印 AttackMontage 的状态
	if (AttackMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] AEnemyBase::Attack - Playing Montage: %s"), *GetName(), *AttackMontage->GetName());
		
		// 播放攻击音效
		if (AttackSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
		}

		// 开启攻击判定 (保底机制：如果蒙太奇里没有加 Notify，这里强制开启)
		if (TraceHitboxComponent)
		{
			// 将攻击命中音效传递给 Hitbox 组件
			if (AttackImpactSound)
			{
				TraceHitboxComponent->HitImpactSound = AttackImpactSound;
			}
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
	if (IsDead()) return;
	if (IsStunned()) return; // [Fix] 眩晕状态下禁止执行攻击结束逻辑（防止重置状态）

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
	if (IsStunned()) return; // [Fix] 眩晕状态下禁止检测目标

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
	// MoveToTarget(PatrolTarget); // [Fix] 移除直接移动，交由行为树
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
	if (IsDead()) return;

	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	
	// 修复：重置仇恨状态，这样下次发现玩家时可以再次播放 AggroMontage
	bHasAggroed = false;
	CombatTarget = nullptr;

	// 停止注视目标
	if (EnemyController)
	{
		EnemyController->ClearFocus(EAIFocusPriority::Gameplay);
	}
	
	// MoveToTarget(PatrolTarget); // [Fix] 移除直接移动，交由行为树
}

void AEnemyBase::ChaseTarget()
{
	if (IsDead()) return;

	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	
	// 强制停止当前移动，确保新的 MoveTo 请求能生效
	if (EnemyController)
	{
		// EnemyController->StopMovement(); // [Fix] 不要停止移动，让行为树接管
		// 锁定注视目标，防止乱转
		EnemyController->SetFocus(CombatTarget);
	}
	
	// MoveToTarget(CombatTarget); // [Fix] 移除直接移动，交由行为树
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
	if (IsDead()) return;

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

bool AEnemyBase::IsStunned()
{
	return EnemyState == EEnemyState::EES_Stunned;
}

void AEnemyBase::StunEnd()
{
	if (IsDead()) return;
	
	UE_LOG(LogTemp, Warning, TEXT("[%s] Stun Recovered."), *GetName());
	
	// 恢复韧性
	CurrentPoise = MaxPoise;
	
	// 恢复状态
	EnemyState = EEnemyState::EES_Chasing;
	
	// 重新开始追击
	CheckCombatTarget();
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

	// 触发战斗BGM切换
	if (AGameStateBase* GameState = GetWorld()->GetGameState())
	{
		if (USceneStateComponent* SceneComp = GameState->FindComponentByClass<USceneStateComponent>())
		{
			SceneComp->OnCombatInitiated();
		}
	}

	// 停止移动 (防止滑步)
	if (EnemyController)
	{
		EnemyController->StopMovement();
		// 停止行为树逻辑 (防止行为树继续下发移动指令，导致“去别的地方”或“滑步”)
		if (UBrainComponent* Brain = EnemyController->GetBrainComponent())
		{
			Brain->StopLogic("Aggro Sensed");
		}
	}
	GetCharacterMovement()->StopMovementImmediately();

	// 清除巡逻计时器 (防止在吼叫时触发巡逻移动，导致“去别的地方再冲过来”的 Bug)
	ClearPatrolTimer();

	// 播放发现音效
	if (AggroSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AggroSound, GetActorLocation());
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
	if (IsDead()) return;

	// 切换到追击状态
	EnemyState = EEnemyState::EES_Chasing;
	
	// 设置更快的速度
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;

	// [Fix] 重新启动行为树逻辑，否则敌人会一直发呆
	if (EnemyController)
	{
		if (UBrainComponent* Brain = EnemyController->GetBrainComponent())
		{
			Brain->RestartLogic();
		}
	}
	
	// 确保朝向目标
	if (CombatTarget)
	{
		ChaseTarget();
	}
}

// ========== 定身术系统实现 ==========

void AEnemyBase::ApplyFreeze(float Duration)
{
	// 不能对死亡的敌人施加定身
	if (IsDead()) return;

	// 如果已经被定身，重置计时器
	if (bIsFrozen)
	{
		GetWorldTimerManager().ClearTimer(FreezeTimer);
	}
	else
	{
		// 保存定身前的状态
		StateBeforeFreeze = EnemyState;
		MovementSpeedBeforeFreeze = GetCharacterMovement()->MaxWalkSpeed;

		// 停止所有行为
		if (EnemyController)
		{
			EnemyController->StopMovement();
			// 暂停行为树
			if (UBrainComponent* Brain = EnemyController->GetBrainComponent())
			{
				Brain->PauseLogic("Frozen");
			}
		}

		// 停止角色移动
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();

		// 暂停动画 - 保持当前帧
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			// 获取当前动画位置（用于恢复时参考）
			if (UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage())
			{
				FrozenAnimPosition = AnimInstance->Montage_GetPosition(CurrentMontage);
			}
			
			// 暂停整个动画蓝图（关键！这会冻结所有动画状态）
			AnimInstance->bReceiveNotifiesFromLinkedInstances = false;
			GetMesh()->bPauseAnims = true;
			GetMesh()->bNoSkeletonUpdate = true;
		}

		// 清除所有计时器（攻击、巡逻、眩晕等）
		ClearAttackTimer();
		ClearPatrolTimer();
		GetWorldTimerManager().ClearTimer(StunTimer);
		GetWorldTimerManager().ClearTimer(AggroTimer);

		// 设置定身状态
		bIsFrozen = true;
		EnemyState = EEnemyState::EES_Frozen;

		// ========== 播放定身音效 ==========
		if (FreezeSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FreezeSound, GetActorLocation());
		}

		// ========== 播放定身特效 (Niagara) ==========
		if (FreezeEffect)
		{
			// 在敌人身上生成持续特效
			ActiveFreezeEffectComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				FreezeEffect,
				GetMesh(),
				NAME_None,
				FVector(0.0f, 0.0f, 50.0f),  // 身体中心偏移
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				false  // 不自动销毁，我们手动控制
			);
		}

		// ========== 显示"定"字 UI ==========
		if (FreezeTextWidgetComponent)
		{
			// 设置位置
			FreezeTextWidgetComponent->SetRelativeLocation(FVector(0.0f, 0.0f, FreezeTextHeightOffset));
			
			// 如果有 Widget 类，设置并显示
			if (FreezeTextWidgetClass)
			{
				FreezeTextWidgetComponent->SetWidgetClass(FreezeTextWidgetClass);
			}
			FreezeTextWidgetComponent->SetVisibility(true);
		}

		UE_LOG(LogTemp, Warning, TEXT("[%s] 被定身！持续 %.1f 秒"), *GetName(), Duration);
	}

	// 设置定身结束计时器
	GetWorldTimerManager().SetTimer(FreezeTimer, this, &AEnemyBase::OnFreezeTimerExpired, Duration, false);
}

void AEnemyBase::RemoveFreeze()
{
	if (!bIsFrozen) return;

	bIsFrozen = false;

	// 清除定身计时器
	GetWorldTimerManager().ClearTimer(FreezeTimer);

	// ========== 隐藏"定"字 UI ==========
	if (FreezeTextWidgetComponent)
	{
		FreezeTextWidgetComponent->SetVisibility(false);
	}

	// ========== 停止定身持续特效 ==========
	if (ActiveFreezeEffectComponent)
	{
		ActiveFreezeEffectComponent->DestroyComponent();
		ActiveFreezeEffectComponent = nullptr;
	}

	// ========== 播放解除定身特效 ==========
	if (UnfreezeEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			UnfreezeEffect,
			GetActorLocation() + FVector(0.0f, 0.0f, 50.0f),
			FRotator::ZeroRotator
		);
	}

	// ========== 播放解除定身音效 ==========
	if (UnfreezeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, UnfreezeSound, GetActorLocation());
	}

	// 恢复动画播放
	if (GetMesh())
	{
		GetMesh()->bPauseAnims = false;
		GetMesh()->bNoSkeletonUpdate = false;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->bReceiveNotifiesFromLinkedInstances = true;
	}

	// 恢复移动能力
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->MaxWalkSpeed = MovementSpeedBeforeFreeze;

	// 恢复行为树
	if (EnemyController)
	{
		if (UBrainComponent* Brain = EnemyController->GetBrainComponent())
		{
			Brain->ResumeLogic("Freeze Ended");
		}
	}

	// 恢复之前的状态（如果之前在追击，继续追击）
	if (StateBeforeFreeze != EEnemyState::EES_NoState && StateBeforeFreeze != EEnemyState::EES_Dead)
	{
		EnemyState = StateBeforeFreeze;

		// 如果之前在战斗中，继续战斗
		if (CombatTarget && (StateBeforeFreeze == EEnemyState::EES_Chasing || 
			StateBeforeFreeze == EEnemyState::EES_Attacking ||
			StateBeforeFreeze == EEnemyState::EES_Engaged))
		{
			ChaseTarget();
		}
	}
	else
	{
		// 默认恢复到警戒/巡逻状态
		StartPatrolling();
	}

	UE_LOG(LogTemp, Warning, TEXT("[%s] 定身解除！恢复到状态: %d"), *GetName(), (int32)EnemyState);
}

void AEnemyBase::OnFreezeTimerExpired()
{
	RemoveFreeze();
}
