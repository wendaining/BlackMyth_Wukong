// 射线扫描 TraceHitbox 组件实现

#include "TraceHitboxComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/CombatComponent.h"
#include "../EnemyBase.h"
#include "../WukongCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "CollisionQueryParams.h"

UTraceHitboxComponent::UTraceHitboxComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	// 默认伤害配置
	DamageInfo.BaseDamage = 10.0f;
	DamageInfo.AttackType = EAttackType::Light;
	DamageInfo.DamageType = EDamageType::Physical;
}

void UTraceHitboxComponent::BeginPlay()
{
	Super::BeginPlay();

	// 设置伤害来源
	DamageInfo.Instigator = GetOwner();
	DamageInfo.DamageCauser = GetOwner();

	// 缓存骨骼网格体组件
	if (ACharacter* OwnerChar = Cast<ACharacter>(GetOwner()))
	{
		CachedMesh = OwnerChar->GetMesh();

		if (CachedMesh.IsValid())
		{
			UE_LOG(LogTemp, Log, TEXT("[TraceHitbox] Found SkeletalMesh on %s"), *GetOwner()->GetName());

			// 检查配置的骨骼/Socket是否存在
			bool bStartExists = DoesBoneOrSocketExist(StartSocketName);
			bool bEndExists = DoesBoneOrSocketExist(EndSocketName);

			if (bStartExists && bEndExists)
			{
				UE_LOG(LogTemp, Log, TEXT("[TraceHitbox] Configured: Start=%s, End=%s, Radius=%.1f"),
					*StartSocketName.ToString(), *EndSocketName.ToString(), TraceRadius);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[TraceHitbox] Bone/Socket not found! Start(%s): %s, End(%s): %s. Will use fallback."),
					*StartSocketName.ToString(), bStartExists ? TEXT("YES") : TEXT("NO"),
					*EndSocketName.ToString(), bEndExists ? TEXT("YES") : TEXT("NO"));
			}
		}
	}

	// 自动查找 CombatComponent
	if (!CachedCombatComponent.IsValid())
	{
		if (AActor* Owner = GetOwner())
		{
			CachedCombatComponent = Owner->FindComponentByClass<UCombatComponent>();
			if (CachedCombatComponent.IsValid())
			{
				UE_LOG(LogTemp, Log, TEXT("[TraceHitbox] Found CombatComponent on %s"), *Owner->GetName());
			}
		}
	}
}

void UTraceHitboxComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 只在激活状态下执行扫描
	if (bIsActive)
	{
		PerformTrace();
	}
}

// ========== 激活控制 ==========

void UTraceHitboxComponent::ActivateTrace()
{
	if (bIsActive)
	{
		return;
	}

	bIsActive = true;
	HitActors.Empty();
	bHasLastFrameData = false;

	// 记录初始位置
	LastStartLocation = GetSocketLocation(StartSocketName);
	LastEndLocation = GetSocketLocation(EndSocketName);
	bHasLastFrameData = true;

	UE_LOG(LogTemp, Log, TEXT("[TraceHitbox] %s Activated (Heavy: %s, Air: %s)"),
		*GetOwner()->GetName(),
		bIsHeavyAttack ? TEXT("YES") : TEXT("NO"),
		bIsAirAttack ? TEXT("YES") : TEXT("NO"));

	OnStateChanged.Broadcast(true);
}

void UTraceHitboxComponent::DeactivateTrace()
{
	if (!bIsActive)
	{
		return;
	}

	bIsActive = false;
	bHasLastFrameData = false;

	// 重置攻击类型
	bIsHeavyAttack = false;
	bIsAirAttack = false;

	UE_LOG(LogTemp, Log, TEXT("[TraceHitbox] %s Deactivated. Hit %d actors."),
		*GetOwner()->GetName(), HitActors.Num());

	OnStateChanged.Broadcast(false);
}

void UTraceHitboxComponent::ClearHitActors()
{
	HitActors.Empty();
}

// ========== CombatComponent 对接 ==========

void UTraceHitboxComponent::SetCombatComponent(UCombatComponent* InCombatComponent)
{
	CachedCombatComponent = InCombatComponent;
}

UCombatComponent* UTraceHitboxComponent::GetCombatComponent() const
{
	return CachedCombatComponent.Get();
}

// ========== 核心扫描逻辑 ==========

void UTraceHitboxComponent::PerformTrace()
{
	if (!GetWorld())
	{
		return;
	}

	// 获取当前帧的位置
	FVector CurrentStart = GetSocketLocation(StartSocketName);
	FVector CurrentEnd = GetSocketLocation(EndSocketName);

	// 配置扫描参数
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bTraceComplex = false;
	QueryParams.bReturnPhysicalMaterial = false;

	TArray<FHitResult> HitResults;
	bool bHit = false;

	// 如果启用插值且有上一帧数据，执行多步扫描以覆盖挥动轨迹
	if (bUseInterpolation && bHasLastFrameData)
	{
		// 步数越多，检测越精确。5步通常足够覆盖快速挥动。
		const int32 NumSteps = 5;
		
		for (int32 i = 0; i < NumSteps; ++i)
		{
			float AlphaStart = (float)i / (float)NumSteps;
			float AlphaEnd = (float)(i + 1) / (float)NumSteps;

			// 1. 扫描 Tip (最重要)
			FVector TipStart = FMath::Lerp(LastEndLocation, CurrentEnd, AlphaStart);
			FVector TipEnd = FMath::Lerp(LastEndLocation, CurrentEnd, AlphaEnd);
			
			TArray<FHitResult> StepHits;
			if (GetWorld()->SweepMultiByChannel(StepHits, TipStart, TipEnd, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(TraceRadius), QueryParams))
			{
				HitResults.Append(StepHits);
				bHit = true;
			}

			// 2. 扫描 Mid (中间点)
			FVector LastMid = (LastStartLocation + LastEndLocation) * 0.5f;
			FVector CurrentMid = (CurrentStart + CurrentEnd) * 0.5f;
			FVector MidStart = FMath::Lerp(LastMid, CurrentMid, AlphaStart);
			FVector MidEnd = FMath::Lerp(LastMid, CurrentMid, AlphaEnd);

			if (GetWorld()->SweepMultiByChannel(StepHits, MidStart, MidEnd, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(TraceRadius), QueryParams))
			{
				HitResults.Append(StepHits);
				bHit = true;
			}

			// 3. 扫描 Handle (握把附近)
			FVector HandleStart = FMath::Lerp(LastStartLocation, CurrentStart, AlphaStart);
			FVector HandleEnd = FMath::Lerp(LastStartLocation, CurrentStart, AlphaEnd);

			if (GetWorld()->SweepMultiByChannel(StepHits, HandleStart, HandleEnd, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(TraceRadius), QueryParams))
			{
				HitResults.Append(StepHits);
				bHit = true;
			}

			// 调试绘制
			if (bDebugDraw)
			{
				DrawDebugLine(GetWorld(), TipStart, TipEnd, bHit ? DebugColorHit : DebugColorMiss, false, DebugDrawDuration, 0, 2.0f);
			}
		}
	}
	else
	{
		// 没有上一帧数据，只扫描当前位置 (Handle -> Tip)
		// 这实际上是检测当前棒身是否与物体重叠
		bHit = GetWorld()->SweepMultiByChannel(
			HitResults,
			CurrentStart,
			CurrentEnd,
			FQuat::Identity,
			TraceChannel,
			FCollisionShape::MakeSphere(TraceRadius),
			QueryParams
		);

		// 绘制调试
		if (bDebugDraw)
		{
			DrawDebugTrace(CurrentStart, CurrentEnd, bHit);
		}
	}

	// 处理命中结果
	if (bHit)
	{
		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (!HitActor)
			{
				continue;
			}

			// 检查是否为有效目标
			if (!IsValidTarget(HitActor))
			{
				continue;
			}

			// 添加到已命中列表
			HitActors.Add(HitActor);

			// 播放命中音效
			if (HitImpactSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, HitImpactSound, Hit.ImpactPoint);
			}

			UE_LOG(LogTemp, Warning, TEXT("[TraceHitbox] %s HIT: %s at (%.1f, %.1f, %.1f)"),
				*GetOwner()->GetName(),
				*HitActor->GetName(),
				Hit.ImpactPoint.X, Hit.ImpactPoint.Y, Hit.ImpactPoint.Z);

			// 广播命中事件
			OnHitDetected.Broadcast(HitActor, Hit);

			// 自动应用伤害
			if (bAutoApplyDamage)
			{
				ApplyDamageToTarget(HitActor, Hit);
			}
		}
	}

	// 更新上一帧位置
	LastStartLocation = CurrentStart;
	LastEndLocation = CurrentEnd;
	bHasLastFrameData = true;
}

bool UTraceHitboxComponent::DoesBoneOrSocketExist(FName Name) const
{
	if (!CachedMesh.IsValid())
	{
		return false;
	}

	// 先检查是否为 Socket
	if (CachedMesh->DoesSocketExist(Name))
	{
		return true;
	}

	// 再检查是否为 Bone
	int32 BoneIndex = CachedMesh->GetBoneIndex(Name);
	return BoneIndex != INDEX_NONE;
}

FVector UTraceHitboxComponent::GetSocketLocation(FName SocketName) const
{
	if (CachedMesh.IsValid())
	{
		// 检查 Socket 或 Bone 是否存在
		bool bIsSocket = CachedMesh->DoesSocketExist(SocketName);
		bool bIsBone = CachedMesh->GetBoneIndex(SocketName) != INDEX_NONE;

		if (bIsSocket || bIsBone)
		{
			// GetSocketLocation 可以同时获取 Socket 和 Bone 的位置
			return CachedMesh->GetSocketLocation(SocketName);
		}

		// 回退：基于手部骨骼计算偏移
		bool bHandIsSocket = CachedMesh->DoesSocketExist(HandBoneName);
		bool bHandIsBone = CachedMesh->GetBoneIndex(HandBoneName) != INDEX_NONE;

		if (bHandIsSocket || bHandIsBone)
		{
			FTransform HandTransform = CachedMesh->GetSocketTransform(HandBoneName);
			FVector HandLocation = HandTransform.GetLocation();
			FVector HandForward = HandTransform.GetRotation().GetForwardVector();

			if (SocketName == StartSocketName)
			{
				// 起点在手部位置
				return HandLocation;
			}
			else if (SocketName == EndSocketName)
			{
				// 终点在前方 WeaponLength 单位
				return HandLocation + HandForward * WeaponLength;
			}
		}
	}

	// 最后回退：返回 Owner 位置
	if (AActor* Owner = GetOwner())
	{
		FVector OwnerLocation = Owner->GetActorLocation();
		FVector OwnerForward = Owner->GetActorForwardVector();

		if (SocketName == StartSocketName)
		{
			return OwnerLocation + FVector(0.0f, 0.0f, 50.0f);  // 大约腰部高度
		}
		else if (SocketName == EndSocketName)
		{
			return OwnerLocation + FVector(0.0f, 0.0f, 50.0f) + OwnerForward * WeaponLength;
		}
	}

	return FVector::ZeroVector;
}

// ========== 目标有效性检查 ==========

bool UTraceHitboxComponent::IsValidTarget(AActor* Target) const
{
	if (!Target || !IsValid(Target))
	{
		return false;
	}

	// 不攻击自己
	if (Target == GetOwner())
	{
		return false;
	}

	// 修复：防止敌人之间互相伤害 (Friendly Fire)
	// 如果攻击者是敌人，且目标也是敌人，则判定为无效目标
	if (GetOwner()->IsA(AEnemyBase::StaticClass()) && Target->IsA(AEnemyBase::StaticClass()))
	{
		return false;
	}

	// 不重复攻击
	if (HitActors.Contains(Target))
	{
		return false;
	}

	// 必须有 HealthComponent 才是可攻击目标
	UHealthComponent* TargetHealth = Target->FindComponentByClass<UHealthComponent>();
	if (!TargetHealth)
	{
		// 没有 HealthComponent 的不是可攻击目标（如地形、道具等）
		// 增加调试日志，帮助排查为什么玩家不扣血
		static bool bWarnedOnce = false;
		if (!bWarnedOnce && Target->GetName().Contains("Wukong"))
		{
			UE_LOG(LogTemp, Warning, TEXT("[TraceHitbox] Target %s REJECTED: No HealthComponent found!"), *Target->GetName());
			bWarnedOnce = true;
		}
		return false;
	}

	// 跳过已死亡的目标
	if (TargetHealth->IsDead())
	{
		return false;
	}

	// 跳过无敌状态的目标
	if (TargetHealth->IsInvincible())
	{
		return false;
	}

	return true;
}

// ========== 伤害应用 ==========

void UTraceHitboxComponent::ApplyDamageToTarget(AActor* Target, const FHitResult& HitResult)
{
	if (!Target)
	{
		return;
	}

	FDamageInfo FinalDamageInfo = DamageInfo;
	float ActualDamage = 0.0f;

	// 通过 CombatComponent 计算伤害
	if (CachedCombatComponent.IsValid())
	{
		ActualDamage = CachedCombatComponent->CalculateFinalDamage(
			FinalDamageInfo,
			bIsHeavyAttack,
			bIsAirAttack
		);
		UE_LOG(LogTemp, Log, TEXT("[TraceHitbox] Damage calculated by CombatComponent: %.1f"), ActualDamage);
	}
	else
	{
		ActualDamage = DamageInfo.GetFinalDamage();
		FinalDamageInfo = DamageInfo;
		UE_LOG(LogTemp, Warning, TEXT("[TraceHitbox] No CombatComponent, using base damage: %.1f"), ActualDamage);
	}

	// 填充命中信息（Trace 提供精确的命中点！）
	FinalDamageInfo.HitLocation = HitResult.ImpactPoint;
	FinalDamageInfo.HitDirection = HitResult.ImpactNormal * -1.0f;
	FinalDamageInfo.Instigator = GetOwner();
	FinalDamageInfo.DamageCauser = GetOwner();

	// 优先处理 EnemyBase
	AEnemyBase* Enemy = Cast<AEnemyBase>(Target);
	if (Enemy)
	{
		if (TryApplyDamageToEnemy(Enemy, ActualDamage))
		{
			if (CachedCombatComponent.IsValid())
			{
				CachedCombatComponent->OnDamageDealt.Broadcast(ActualDamage, Target, FinalDamageInfo.bIsCritical);
			}
			return;
		}
	}

	// 优先处理 WukongCharacter (确保播放受击动画)
	AWukongCharacter* Wukong = Cast<AWukongCharacter>(Target);
	if (Wukong)
	{
		Wukong->ReceiveDamage(ActualDamage, GetOwner());

		UE_LOG(LogTemp, Warning, TEXT("[TraceHitbox] Applied %.1f damage to Wukong %s via ReceiveDamage"),
			ActualDamage, *Wukong->GetName());

		if (CachedCombatComponent.IsValid())
		{
			CachedCombatComponent->OnDamageDealt.Broadcast(ActualDamage, Target, FinalDamageInfo.bIsCritical);
		}
		return;
	}

	// 通过 HealthComponent
	UHealthComponent* TargetHealth = Target->FindComponentByClass<UHealthComponent>();
	if (TargetHealth)
	{
		TargetHealth->TakeDamage(ActualDamage, GetOwner());

		UE_LOG(LogTemp, Warning, TEXT("[TraceHitbox] Applied %.1f damage to %s (Remaining: %.1f)"),
			ActualDamage, *Target->GetName(), TargetHealth->GetCurrentHealth());

		if (CachedCombatComponent.IsValid())
		{
			CachedCombatComponent->OnDamageDealt.Broadcast(ActualDamage, Target, FinalDamageInfo.bIsCritical);
		}
		return;
	}

	// 后备使用UE内置伤害
	UGameplayStatics::ApplyDamage(
		Target,
		ActualDamage,
		GetOwner()->GetInstigatorController(),
		GetOwner(),
		nullptr
	);

	UE_LOG(LogTemp, Log, TEXT("[TraceHitbox] Applied %.1f UE damage to %s (no HealthComponent)"),
		ActualDamage, *Target->GetName());
}

bool UTraceHitboxComponent::TryApplyDamageToEnemy(AEnemyBase* Enemy, float FinalDamage)
{
	if (!Enemy)
	{
		return false;
	}

	Enemy->ReceiveDamage(FinalDamage, GetOwner());

	UE_LOG(LogTemp, Warning, TEXT("[TraceHitbox] Applied %.1f damage to Enemy %s via ReceiveDamage"),
		FinalDamage, *Enemy->GetName());

	return true;
}

// ========== 调试绘制 ==========

void UTraceHitboxComponent::DrawDebugTrace(const FVector& Start, const FVector& End, bool bHit)
{
	if (!GetWorld())
	{
		return;
	}

	FColor DrawColor = bHit ? DebugColorHit : DebugColorMiss;

	// 绘制扫描线
	DrawDebugLine(GetWorld(), Start, End, DrawColor, false, DebugDrawDuration, 0, 2.0f);

	// 绘制起点和终点的球
	DrawDebugSphere(GetWorld(), Start, TraceRadius, 8, DrawColor, false, DebugDrawDuration, 0, 1.0f);
	DrawDebugSphere(GetWorld(), End, TraceRadius, 8, DrawColor, false, DebugDrawDuration, 0, 1.0f);

	// 绘制扫描胶囊轮廓（可视化整个扫描区域）
	FVector MidPoint = (Start + End) / 2.0f;
	float HalfHeight = FVector::Dist(Start, End) / 2.0f;

	if (HalfHeight > 0.1f)  // 避免零长度
	{
		FVector Direction = (End - Start).GetSafeNormal();
		FQuat CapsuleRotation = FQuat::FindBetweenVectors(FVector::UpVector, Direction);

		DrawDebugCapsule(
			GetWorld(),
			MidPoint,
			HalfHeight,
			TraceRadius,
			CapsuleRotation,
			DrawColor,
			false,
			DebugDrawDuration,
			0,
			1.0f
		);
	}
}
