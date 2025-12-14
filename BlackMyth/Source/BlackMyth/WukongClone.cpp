// 悟空分身类实现

#include "WukongClone.h"
#include "WukongCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/CombatComponent.h"
#include "Components/TeamComponent.h"
#include "Combat/TraceHitboxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnemyBase.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Engine/SkeletalMesh.h"

AWukongClone::AWukongClone()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建生命组件
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	// 创建战斗组件
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	// 创建阵营组件，设置为玩家阵营（这样敌人会攻击分身）
	TeamComponent = CreateDefaultSubobject<UTeamComponent>(TEXT("TeamComponent"));
	TeamComponent->SetTeam(ETeam::Player);

	// 创建武器碰撞检测组件
	WeaponTraceHitbox = CreateDefaultSubobject<UTraceHitboxComponent>(TEXT("WeaponTraceHitbox"));

	// 创建AI感知刺激源组件，这样敌人的感知系统能检测到分身
	UAIPerceptionStimuliSourceComponent* StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
	StimuliSource->RegisterForSense(TSubclassOf<UAISense>(UAISense_Sight::StaticClass()));
	StimuliSource->bAutoRegister = true;

	// 设置胶囊体大小
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// 配置移动组件
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = true;
		Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
		Movement->MaxWalkSpeed = MoveSpeed;
	}

	// 默认使用AI控制
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AWukongClone::BeginPlay()
{
	Super::BeginPlay();

	// 绑定死亡事件
	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &AWukongClone::HandleDeath);
	}
}

void AWukongClone::HandleDeath(AActor* Killer)
{
	// 死亡时消失
	Disappear();
}

void AWukongClone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("WukongClone::Tick - Not initialized yet!"));
		return;
	}

	// 更新攻击冷却
	if (AttackCooldownTimer > 0.0f)
	{
		AttackCooldownTimer -= DeltaTime;
	}

	// 如果正在攻击，等待攻击结束
	if (bIsAttacking)
	{
		return;
	}

	// 寻找目标
	if (!CurrentTarget || !IsValid(CurrentTarget))
	{
		CurrentTarget = FindNearestEnemy();
		if (CurrentTarget)
		{
			UE_LOG(LogTemp, Log, TEXT("WukongClone: Found target: %s"), *CurrentTarget->GetName());
		}
	}

	// 如果有目标，执行AI逻辑
	if (CurrentTarget && IsValid(CurrentTarget))
	{
		float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

		// 检查目标是否死亡（如果是敌人）
		if (AEnemyBase* Enemy = Cast<AEnemyBase>(CurrentTarget))
		{
			if (Enemy->IsDead())
			{
				CurrentTarget = nullptr;
				return;
			}
		}

		if (DistanceToTarget <= AttackRange)
		{
			// 在攻击范围内，面向目标并攻击
			FVector Direction = CurrentTarget->GetActorLocation() - GetActorLocation();
			Direction.Z = 0.0f;
			if (!Direction.IsNearlyZero())
			{
				FRotator TargetRotation = Direction.Rotation();
				SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 10.0f));
			}

			// 攻击
			if (AttackCooldownTimer <= 0.0f)
			{
				UE_LOG(LogTemp, Log, TEXT("WukongClone: Attacking target!"));
				PerformAttack();
			}
		}
		else
		{
			// 不在攻击范围，移动到目标
			MoveToTarget(CurrentTarget);
		}
	}
	else
	{
		// 没有目标，跟随召唤者
		if (CloneOwner && IsValid(CloneOwner))
		{
			float DistanceToOwner = FVector::Dist(GetActorLocation(), CloneOwner->GetActorLocation());
			if (DistanceToOwner > 300.0f)
			{
				MoveToTarget(CloneOwner);
			}
		}
	}
}

void AWukongClone::InitializeClone(AActor* InOwner, float InLifetime)
{
	CloneOwner = InOwner;
	Lifetime = InLifetime;
	bIsInitialized = true;

	// 从召唤者复制外观和动画
	if (ACharacter* OwnerChar = Cast<ACharacter>(InOwner))
	{
		if (USkeletalMeshComponent* OwnerMesh = OwnerChar->GetMesh())
		{
			USkeletalMeshComponent* MyMesh = GetMesh();
			if (MyMesh)
			{
				// 复制骨骼网格
				if (USkeletalMesh* SkelMesh = OwnerMesh->GetSkeletalMeshAsset())
				{
					MyMesh->SetSkeletalMesh(SkelMesh);
				}
				
				// 复制动画蓝图
				if (UClass* AnimClass = OwnerMesh->GetAnimClass())
				{
					MyMesh->SetAnimInstanceClass(AnimClass);
				}
				
				// 设置相同的相对变换
				MyMesh->SetRelativeLocation(OwnerMesh->GetRelativeLocation());
				MyMesh->SetRelativeRotation(OwnerMesh->GetRelativeRotation());
				
				UE_LOG(LogTemp, Log, TEXT("WukongClone: Copied mesh and animation from owner"));
				
				// 配置武器碰撞检测组件
				if (WeaponTraceHitbox)
				{
					WeaponTraceHitbox->SetMeshToTrace(MyMesh);
					WeaponTraceHitbox->SetStartSocket(FName("weapon_r"));  // 握把位置
					WeaponTraceHitbox->SetEndSocket(FName("weapon_tou")); // 武器前端
					WeaponTraceHitbox->SetTraceRadius(7.0f);
					
					if (CombatComponent)
					{
						WeaponTraceHitbox->SetCombatComponent(CombatComponent);
					}
					
					UE_LOG(LogTemp, Log, TEXT("WukongClone: WeaponTraceHitbox configured"));
				}
			}
		}
	}

	// 设置生命周期计时器
	GetWorldTimerManager().SetTimer(
		LifetimeTimerHandle,
		this,
		&AWukongClone::OnLifetimeExpired,
		Lifetime,
		false
	);

	UE_LOG(LogTemp, Log, TEXT("WukongClone: Initialized with lifetime %.1f seconds"), Lifetime);
}

void AWukongClone::PerformAttack()
{
	// 先设置攻击状态，防止重复触发
	bIsAttacking = true;
	AttackCooldownTimer = AttackCooldown;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("WukongClone: No AnimInstance!"));
		bIsAttacking = false;
		return;
	}

	// 尝试播放攻击蒙太奇
	UAnimMontage* MontageToPlay = nullptr;
	
	if (AttackMontages.Num() > 0)
	{
		// 使用配置的攻击蒙太奇
		MontageToPlay = AttackMontages[CurrentAttackIndex % AttackMontages.Num()];
		CurrentAttackIndex++;
	}
	else
	{
		// 后备方案：尝试从召唤者获取攻击蒙太奇
		if (AWukongCharacter* WukongOwner = Cast<AWukongCharacter>(CloneOwner))
		{
			// 使用悟空的攻击蒙太奇
			if (WukongOwner->AttackMontage1)
			{
				MontageToPlay = WukongOwner->AttackMontage1;
			}
		}
	}

	if (MontageToPlay)
	{
		float Duration = AnimInstance->Montage_Play(MontageToPlay, 1.2f);
		UE_LOG(LogTemp, Log, TEXT("WukongClone: Playing attack montage, duration=%.2f"), Duration);

		// 激活武器碰撞检测
		if (WeaponTraceHitbox)
		{
			WeaponTraceHitbox->ActivateTrace();
		}

		// 设置攻击结束回调
		FTimerHandle AttackEndTimer;
		GetWorldTimerManager().SetTimer(
			AttackEndTimer,
			[this]()
			{
				bIsAttacking = false;
				// 停用武器碰撞检测
				if (WeaponTraceHitbox)
				{
					WeaponTraceHitbox->DeactivateTrace();
					WeaponTraceHitbox->ClearHitActors();
				}
			},
			Duration * 0.8f,
			false
		);
	}
	else
	{
		// 没有蒙太奇可用，模拟攻击动作（只是延迟）
		UE_LOG(LogTemp, Warning, TEXT("WukongClone: No attack montage available, simulating attack"));
		
		FTimerHandle AttackEndTimer;
		GetWorldTimerManager().SetTimer(
			AttackEndTimer,
			[this]()
			{
				bIsAttacking = false;
			},
			1.0f, // 1秒后结束攻击
			false
		);
	}
}

void AWukongClone::Disappear()
{
	UE_LOG(LogTemp, Log, TEXT("WukongClone: Disappearing..."));

	// 清除计时器
	GetWorldTimerManager().ClearTimer(LifetimeTimerHandle);

	// 播放消失特效
	if (DisappearEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			DisappearEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}

	// 播放消失声音
	if (DisappearSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			DisappearSound,
			GetActorLocation()
		);
	}

	// 销毁自身
	Destroy();
}

void AWukongClone::OnLifetimeExpired()
{
	UE_LOG(LogTemp, Log, TEXT("WukongClone: Lifetime expired"));
	Disappear();
}

AActor* AWukongClone::FindNearestEnemy()
{
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyBase::StaticClass(), FoundEnemies);

	AActor* NearestEnemy = nullptr;
	float NearestDistance = DetectionRange;

	for (AActor* Enemy : FoundEnemies)
	{
		// 检查敌人是否存活
		if (AEnemyBase* EnemyBase = Cast<AEnemyBase>(Enemy))
		{
			if (EnemyBase->IsDead())
			{
				continue;
			}
		}

		float Distance = FVector::Dist(GetActorLocation(), Enemy->GetActorLocation());
		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			NearestEnemy = Enemy;
		}
	}

	return NearestEnemy;
}

void AWukongClone::MoveToTarget(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (!Movement)
	{
		return;
	}

	// 计算移动方向
	FVector MyLocation = GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	FVector Direction = TargetLocation - MyLocation;
	Direction.Z = 0.0f;
	
	float Distance = Direction.Size();
	if (Distance < 10.0f) // 太近了就不移动
	{
		return;
	}
	
	Direction.Normalize();

	// 直接使用 AddMovementInput 配合正确的控制旋转
	// 关键：需要设置控制旋转让角色知道该往哪走
	if (Controller)
	{
		Controller->SetControlRotation(Direction.Rotation());
	}
	
	// 使用内置的移动输入系统
	AddMovementInput(Direction, 1.0f);

	// 面向移动方向（平滑旋转）
	FRotator CurrentRotation = GetActorRotation();
	FRotator TargetRotation = Direction.Rotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 10.0f);
	SetActorRotation(NewRotation);
}
