// Hitbox组件实现

#include "HitboxComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/CombatComponent.h"
#include "../EnemyBase.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UHitboxComponent::UHitboxComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// 设置碰撞通道和响应
	SetCollisionEnabled(ECollisionEnabled::NoCollision);  // 默认禁用
	SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	SetGenerateOverlapEvents(true);

	// 默认伤害配置
	DamageInfo.BaseDamage = 10.0f;
	DamageInfo.AttackType = EAttackType::Light;
	DamageInfo.DamageType = EDamageType::Physical;
}

void UHitboxComponent::BeginPlay()
{
	Super::BeginPlay();

	// TODO [MemberA]: 以下尺寸需要根据悟空模型的武器骨骼调整
	// 当前使用临时值，待模型确定后修改
	SetBoxExtent(FVector(50.0f, 10.0f, 10.0f));

	// TODO [MemberA]: 以下相对位置需要附加到武器骨骼上
	// 当前挂载在 RootComponent，待骨骼名称确定后修改
	// 示例：
	// if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	// {
	//     AttachToComponent(OwnerCharacter->GetMesh(),
	//         FAttachmentTransformRules::SnapToTargetNotIncludingScale,
	//         FName("weapon_socket"));
	// }

	// 不可见
	SetVisibility(false);
	SetHiddenInGame(true);

	// 设置伤害来源
	DamageInfo.Instigator = GetOwner();
	DamageInfo.DamageCauser = GetOwner();

	// 绑定碰撞事件
	OnComponentBeginOverlap.AddDynamic(this, &UHitboxComponent::OnHitboxBeginOverlap);

	// 确保初始状态禁用
	DeactivateHitbox();

	// 自动查找 Owner 的 CombatComponent
	if (!CachedCombatComponent.IsValid())
	{
		if (AActor* Owner = GetOwner())
		{
			CachedCombatComponent = Owner->FindComponentByClass<UCombatComponent>();
			if (CachedCombatComponent.IsValid())
			{
				UE_LOG(LogTemp, Log, TEXT("[Hitbox] Found CombatComponent on %s"), *Owner->GetName());
			}
		}
	}
}

void UHitboxComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 更新命中闪烁
	if (HitFlashTimer > 0.0f)
	{
		HitFlashTimer -= DeltaTime;
	}

	// 绘制调试形状
	if (bDebugDraw)
	{
		DrawDebugHitbox();
	}
}

// ========== 激活控制 ==========

void UHitboxComponent::ActivateHitbox()
{
	if (bIsActive)
	{
		return;
	}

	bIsActive = true;
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitActors.Empty();  // 清除上次的命中记录

	UE_LOG(LogTemp, Log, TEXT("[Hitbox] %s Activated (Heavy: %s, Air: %s)"),
		*GetOwner()->GetName(),
		bIsHeavyAttack ? TEXT("YES") : TEXT("NO"),
		bIsAirAttack ? TEXT("YES") : TEXT("NO"));

	OnHitboxStateChanged.Broadcast(true);
}

void UHitboxComponent::DeactivateHitbox()
{
	if (!bIsActive)
	{
		return;
	}

	bIsActive = false;
	SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 重置攻击类型状态
	bIsHeavyAttack = false;
	bIsAirAttack = false;

	UE_LOG(LogTemp, Log, TEXT("[Hitbox] %s Deactivated. Hit %d actors."), *GetOwner()->GetName(), HitActors.Num());

	OnHitboxStateChanged.Broadcast(false);
}

void UHitboxComponent::SetHitboxActive(bool bActive)
{
	if (bActive)
	{
		ActivateHitbox();
	}
	else
	{
		DeactivateHitbox();
	}
}

void UHitboxComponent::ClearHitActors()
{
	HitActors.Empty();
}

// ========== CombatComponent 对接 ==========

void UHitboxComponent::SetCombatComponent(UCombatComponent* InCombatComponent)
{
	CachedCombatComponent = InCombatComponent;
}

UCombatComponent* UHitboxComponent::GetCombatComponent() const
{
	return CachedCombatComponent.Get();
}

// ========== 目标有效性检查（MemberC 核心逻辑） ==========

bool UHitboxComponent::IsValidTarget(AActor* Target) const
{
	// 检查 Actor 基础有效性
	if (Target == nullptr || !IsValid(Target))
	{
		return false;
	}

	// 不攻击自己
	if (bIgnoreOwner && Target == GetOwner())
	{
		return false;
	}

	// 不重复攻击同一目标
	if (HitActors.Contains(Target))
	{
		return false;
	}

	// 检查目标的 HealthComponent 状态
	UHealthComponent* TargetHealth = Target->FindComponentByClass<UHealthComponent>();
	if (TargetHealth)
	{
		// 跳过已死亡的目标（避免鞭尸）
		if (TargetHealth->IsDead())
		{
			UE_LOG(LogTemp, Verbose, TEXT("[Hitbox] Skipped dead target: %s"), *Target->GetName());
			return false;
		}

		// 跳过无敌状态的目标
		if (TargetHealth->IsInvincible())
		{
			UE_LOG(LogTemp, Verbose, TEXT("[Hitbox] Skipped invincible target: %s"), *Target->GetName());
			return false;
		}
	}

	// 检查碰撞是否启用
	UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Target->GetRootComponent());
	if (RootPrimitive)
	{
		if (RootPrimitive->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
		{
			UE_LOG(LogTemp, Verbose, TEXT("[Hitbox] Skipped no-collision target: %s"), *Target->GetName());
			return false;
		}
	}

	// TODO : 未来添加 TeamComponent 阵营检查

	return true;
}

// ========== 碰撞处理 ==========

void UHitboxComponent::OnHitboxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 不激活时不处理
	if (!bIsActive)
	{
		return;
	}

	// 检查有效性
	if (!IsValidTarget(OtherActor))
	{
		return;
	}

	// 添加到已命中列表
	HitActors.Add(OtherActor);
	HitFlashTimer = 0.2f;  // 命中闪烁

	UE_LOG(LogTemp, Warning, TEXT("[Hitbox] %s HIT: %s"), *GetOwner()->GetName(), *OtherActor->GetName());

	// 构建命中信息
	FHitResult HitResult = SweepResult;
	if (!HitResult.bBlockingHit)
	{
		// 如果没有详细命中信息，手动填充
		HitResult.ImpactPoint = OtherActor->GetActorLocation();
		HitResult.ImpactNormal = (GetOwner()->GetActorLocation() - OtherActor->GetActorLocation()).GetSafeNormal();
	}

	// 广播命中事件（供外部监听：顿帧、飙血特效等）
	OnHitDetected.Broadcast(OtherActor, HitResult);

	// 自动应用伤害
	if (bAutoApplyDamage)
	{
		ApplyDamageToTarget(OtherActor, HitResult);
	}
}

// ========== 伤害应用 ==========

void UHitboxComponent::ApplyDamageToTarget(AActor* Target, const FHitResult& HitResult)
{
	if (!Target)
	{
		return;
	}

	// ========== 通过 CombatComponent 计算伤害 ==========
	FDamageInfo FinalDamageInfo = DamageInfo;  // 从默认配置开始
	float ActualDamage = 0.0f;

	if (CachedCombatComponent.IsValid())
	{
		// 使用 CombatComponent 进行完整伤害计算
		ActualDamage = CachedCombatComponent->CalculateFinalDamage(
			FinalDamageInfo,
			bIsHeavyAttack,
			bIsAirAttack
		);

		UE_LOG(LogTemp, Log, TEXT("[Hitbox] Damage calculated by CombatComponent: %.1f"), ActualDamage);
	}
	else
	{
		// 使用 Hitbox 自身配置的基础伤害
		ActualDamage = DamageInfo.GetFinalDamage();
		FinalDamageInfo = DamageInfo;
		UE_LOG(LogTemp, Warning, TEXT("[Hitbox] No CombatComponent found, using base damage: %.1f"), ActualDamage);
	}

	// ========== 填充命中位置和方向 ==========
	FinalDamageInfo.HitLocation = HitResult.ImpactPoint;
	FinalDamageInfo.HitDirection = HitResult.ImpactNormal * -1.0f;  // 受击方向 = 法线反向
	FinalDamageInfo.Instigator = GetOwner();
	FinalDamageInfo.DamageCauser = GetOwner();

	// ========== 优先尝试作为 EnemyBase 处理 ==========
	AEnemyBase* Enemy = Cast<AEnemyBase>(Target);
	if (Enemy)
	{
		if (TryApplyDamageToEnemy(Enemy, ActualDamage))
		{
			// 广播伤害造成事件
			if (CachedCombatComponent.IsValid())
			{
				CachedCombatComponent->OnDamageDealt.Broadcast(ActualDamage, Target, FinalDamageInfo.bIsCritical);
			}
			return;
		}
	}

	// ========== 通过 HealthComponent 应用伤害 ==========
	UHealthComponent* TargetHealth = Target->FindComponentByClass<UHealthComponent>();
	if (TargetHealth)
	{
		TargetHealth->TakeDamage(ActualDamage, GetOwner());

		UE_LOG(LogTemp, Warning, TEXT("[Hitbox] Applied %.1f damage to %s via HealthComponent (Remaining: %.1f)"),
			ActualDamage, *Target->GetName(), TargetHealth->GetCurrentHealth());

		// 广播伤害造成事件
		if (CachedCombatComponent.IsValid())
		{
			CachedCombatComponent->OnDamageDealt.Broadcast(ActualDamage, Target, FinalDamageInfo.bIsCritical);
		}
		return;
	}

	// ========== 后备最后方案：使用 UE 内置伤害系统 ==========
	UGameplayStatics::ApplyDamage(
		Target,
		ActualDamage,
		GetOwner()->GetInstigatorController(),
		GetOwner(),
		nullptr
	);

	UE_LOG(LogTemp, Log, TEXT("[Hitbox] Applied %.1f UE damage to %s (no HealthComponent)"),
		ActualDamage, *Target->GetName());
}

bool UHitboxComponent::TryApplyDamageToEnemy(AEnemyBase* Enemy, float FinalDamage)
{
	if (!Enemy)
	{
		return false;
	}

	// 调用 EnemyBase 受伤接口
	// 触发敌人的受击动画
	Enemy->ReceiveDamage(FinalDamage, GetOwner());

	UE_LOG(LogTemp, Warning, TEXT("[Hitbox] Applied %.1f damage to Enemy %s via ReceiveDamage"),
		FinalDamage, *Enemy->GetName());

	return true;
}

// ========== 调试绘制 ==========

void UHitboxComponent::DrawDebugHitbox()
{
	if (!GetWorld())
	{
		return;
	}

	// 确定颜色
	FColor DrawColor;
	if (HitFlashTimer > 0.0f)
	{
		DrawColor = DebugColorHit;
	}
	else if (bIsActive)
	{
		DrawColor = DebugColorActive;
	}
	else
	{
		DrawColor = DebugColorInactive;
	}

	// 获取世界变换
	FVector Center = GetComponentLocation();
	FQuat Rotation = GetComponentQuat();
	FVector Extent = GetScaledBoxExtent();

	// 绘制 Box
	DrawDebugBox(
		GetWorld(),
		Center,
		Extent,
		Rotation,
		DrawColor,
		false,  // bPersistent
		0.0f,   // LifeTime (0 = 一帧)
		0,      // DepthPriority
		bIsActive ? 2.0f : 1.0f  // Thickness
	);

	// 激活时绘制方向箭头
	if (bIsActive)
	{
		FVector Forward = Rotation.GetForwardVector() * Extent.X;
		DrawDebugDirectionalArrow(
			GetWorld(),
			Center,
			Center + Forward,
			20.0f,
			DrawColor,
			false,
			0.0f,
			0,
			2.0f
		);
	}
}
