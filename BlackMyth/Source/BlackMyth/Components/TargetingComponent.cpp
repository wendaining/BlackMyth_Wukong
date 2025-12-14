// 目标锁定组件实现

#include "TargetingComponent.h"
#include "HealthComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"

UTargetingComponent::UTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UTargetingComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 获取玩家控制器
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		OwnerController = Cast<APlayerController>(OwnerPawn->GetController());
	}
}

void UTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsTargeting)
	{
		// 验证当前目标是否有效
		ValidateCurrentTarget(DeltaTime);

		// 更新摄像机朝向
		if (LockedTarget)
		{
			UpdateCameraToTarget(DeltaTime);
			UpdateTargetIndicator();
		}
	}
}

void UTargetingComponent::ToggleLockOn()
{
	if (bIsTargeting)
	{
		// 当前已锁定，解除锁定
		ClearTarget();
		UE_LOG(LogTemp, Log, TEXT("[TargetingComponent] Lock-on disabled"));
	}
	else
	{
		// 尝试锁定目标
		AActor* NewTarget = FindBestTarget();
		if (NewTarget)
		{
			LockedTarget = NewTarget;
			bIsTargeting = true;
			LineOfSightLostTimer = 0.0f;

			// 绑定目标死亡事件
			if (UHealthComponent* TargetHealth = NewTarget->FindComponentByClass<UHealthComponent>())
			{
				TargetHealth->OnDeath.AddDynamic(this, &UTargetingComponent::OnTargetDeath);
			}

			// 创建锁定指示器
			CreateTargetIndicator();

			// 广播目标变更
			OnTargetChanged.Broadcast(NewTarget);
			
			UE_LOG(LogTemp, Log, TEXT("[TargetingComponent] Locked on to: %s"), *NewTarget->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("[TargetingComponent] No valid target found"));
		}
	}
}
void UTargetingComponent::SwitchTarget(bool bRight)
{
	if (!bIsTargeting || !LockedTarget)
	{
		return;
	}

	TArray<AActor*> AllTargets = FindAllTargets();
	
	if (AllTargets.Num() <= 1)
	{
		return; // 没有其他目标可切换
	}

	// 移除当前目标
	AllTargets.Remove(LockedTarget);

	// 根据方向计算每个目标的得分
	AActor* BestTarget = nullptr;
	float BestScore = -FLT_MAX;

	for (AActor* Target : AllTargets)
	{
		float Score = CalculateTargetScore(Target, bRight);
		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Target;
		}
	}

	if (BestTarget && BestTarget != LockedTarget)
	{
		// 解绑旧目标的死亡事件
		if (UHealthComponent* OldHealth = LockedTarget->FindComponentByClass<UHealthComponent>())
		{
			OldHealth->OnDeath.RemoveDynamic(this, &UTargetingComponent::OnTargetDeath);
		}

		// 切换到新目标
		LockedTarget = BestTarget;
		LineOfSightLostTimer = 0.0f;

		// 绑定新目标的死亡事件
		if (UHealthComponent* NewHealth = BestTarget->FindComponentByClass<UHealthComponent>())
		{
			NewHealth->OnDeath.AddDynamic(this, &UTargetingComponent::OnTargetDeath);
		}

		OnTargetChanged.Broadcast(BestTarget);
		UE_LOG(LogTemp, Log, TEXT("[TargetingComponent] Switched target to: %s"), *BestTarget->GetName());
	}
}

void UTargetingComponent::ClearTarget()
{
	if (LockedTarget)
	{
		// 解绑死亡事件
		if (UHealthComponent* TargetHealth = LockedTarget->FindComponentByClass<UHealthComponent>())
		{
			TargetHealth->OnDeath.RemoveDynamic(this, &UTargetingComponent::OnTargetDeath);
		}
	}

	// 销毁锁定指示器
	DestroyTargetIndicator();

	LockedTarget = nullptr;
	bIsTargeting = false;
	LineOfSightLostTimer = 0.0f;

	OnTargetLost.Broadcast();
}

FVector UTargetingComponent::GetDirectionToTarget() const
{
	if (!LockedTarget || !GetOwner())
	{
		return FVector::ForwardVector;
	}

	FVector OwnerLocation = GetOwner()->GetActorLocation();
	FVector TargetLocation = LockedTarget->GetActorLocation();
	
	FVector Direction = (TargetLocation - OwnerLocation).GetSafeNormal2D();
	return Direction.IsNearlyZero() ? FVector::ForwardVector : Direction;
}

float UTargetingComponent::GetDistanceToTarget() const
{
	if (!LockedTarget || !GetOwner())
	{
		return 0.0f;
	}

	return FVector::Dist(GetOwner()->GetActorLocation(), LockedTarget->GetActorLocation());
}

AActor* UTargetingComponent::FindBestTarget() const
{
	TArray<AActor*> AllTargets = FindAllTargets();
	
	if (AllTargets.Num() == 0)
	{
		return nullptr;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	// 获取摄像机前方向（优先使用摄像机方向，而非角色朝向）
	FVector LookDirection = Owner->GetActorForwardVector();
	if (OwnerController)
	{
		FRotator ControlRotation = OwnerController->GetControlRotation();
		LookDirection = ControlRotation.Vector();
	}

	FVector OwnerLocation = Owner->GetActorLocation();
	
	AActor* BestTarget = nullptr;
	float BestScore = -FLT_MAX;

	for (AActor* Target : AllTargets)
	{
		FVector ToTarget = Target->GetActorLocation() - OwnerLocation;
		float Distance = ToTarget.Size();
		FVector DirectionToTarget = ToTarget.GetSafeNormal();

		// 计算与视线的夹角
		float DotProduct = FVector::DotProduct(LookDirection.GetSafeNormal2D(), DirectionToTarget.GetSafeNormal2D());
		
		// 综合评分：距离越近、越靠近屏幕中央得分越高
		// DotProduct 范围 [-1, 1]，1 表示正前方
		float AngleScore = DotProduct * 100.0f; // 角度权重
		float DistanceScore = (TargetingDistance - Distance) / TargetingDistance * 50.0f; // 距离权重

		float Score = AngleScore + DistanceScore;

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Target;
		}
	}

	return BestTarget;
}

TArray<AActor*> UTargetingComponent::FindAllTargets() const
{
	TArray<AActor*> ValidTargets;
	
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return ValidTargets;
	}

	FVector OwnerLocation = Owner->GetActorLocation();

	// 获取视线方向
	FVector LookDirection = Owner->GetActorForwardVector();
	if (OwnerController)
	{
		FRotator ControlRotation = OwnerController->GetControlRotation();
		LookDirection = ControlRotation.Vector();
	}

	// 收集所有潜在目标
	TArray<AActor*> PotentialTargets;

	if (TargetableClasses.Num() > 0)
	{
		// 使用指定的类
		for (TSubclassOf<AActor> TargetClass : TargetableClasses)
		{
			TArray<AActor*> FoundActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), TargetClass, FoundActors);
			PotentialTargets.Append(FoundActors);
		}
	}
	else
	{
		// 默认：查找所有带 HealthComponent 的 Actor
		TArray<AActor*> AllActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
		
		for (AActor* Actor : AllActors)
		{
			if (Actor->FindComponentByClass<UHealthComponent>())
			{
				PotentialTargets.Add(Actor);
			}
		}
	}

	// 筛选有效目标
	for (AActor* Target : PotentialTargets)
	{
		// 排除自己
		if (Target == Owner)
		{
			continue;
		}

		// 检查标签
		if (TargetTag != NAME_None && !Target->ActorHasTag(TargetTag))
		{
			continue;
		}

		// 检查距离
		float Distance = FVector::Dist(OwnerLocation, Target->GetActorLocation());
		if (Distance > TargetingDistance)
		{
			continue;
		}

		// 检查角度（是否在前方扇形区域内）
		FVector ToTarget = (Target->GetActorLocation() - OwnerLocation).GetSafeNormal2D();
		float DotProduct = FVector::DotProduct(LookDirection.GetSafeNormal2D(), ToTarget);
		float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(DotProduct));
		
		if (AngleDegrees > TargetingAngle)
		{
			continue;
		}

		// 检查是否存活
		if (!IsTargetAlive(Target))
		{
			continue;
		}

		// 检查视线（可选）
		if (bCheckLineOfSight && !HasLineOfSight(Target))
		{
			continue;
		}

		ValidTargets.Add(Target);
	}

	return ValidTargets;
}

bool UTargetingComponent::IsTargetValid(AActor* Target) const
{
	if (!Target || !Target->IsValidLowLevel() || Target->IsPendingKillPending())
	{
		return false;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return false;
	}

	// 检查距离
	float Distance = FVector::Dist(Owner->GetActorLocation(), Target->GetActorLocation());
	if (Distance > LoseTargetDistance)
	{
		return false;
	}

	// 检查是否存活
	if (!IsTargetAlive(Target))
	{
		return false;
	}

	return true;
}

bool UTargetingComponent::HasLineOfSight(AActor* Target) const
{
	if (!Target || !GetOwner())
	{
		return false;
	}

	FVector Start = GetOwner()->GetActorLocation() + FVector(0, 0, 50.0f); // 从眼睛高度
	FVector End = Target->GetActorLocation() + FVector(0, 0, TargetHeightOffset);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(Target);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	// 如果没有命中任何东西，说明视线畅通
	return !bHit;
}

bool UTargetingComponent::IsTargetAlive(AActor* Target) const
{
	if (!Target)
	{
		return false;
	}

	if (UHealthComponent* HealthComp = Target->FindComponentByClass<UHealthComponent>())
	{
		return !HealthComp->IsDead();
	}

	// 没有 HealthComponent，假设活着
	return true;
}

void UTargetingComponent::UpdateCameraToTarget(float DeltaTime)
{
	if (!LockedTarget || !OwnerController)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// 只计算水平方向（Yaw）的锁定，完全不干涉上下视角（Pitch）
	FVector TargetLocation = LockedTarget->GetActorLocation();
	FVector OwnerLocation = Owner->GetActorLocation();
	
	// 只在水平面上计算方向
	FVector ToTarget2D = (TargetLocation - OwnerLocation).GetSafeNormal2D();
	float IdealYaw = FMath::Atan2(ToTarget2D.Y, ToTarget2D.X) * (180.0f / PI);

	// 获取当前控制器旋转
	FRotator CurrentRotation = OwnerController->GetControlRotation();

	// 计算 Yaw 差值
	float YawDelta = FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, IdealYaw);
	
	// 只有超出自由范围才进行修正
	float YawCorrection = 0.0f;
	if (FMath::Abs(YawDelta) > YawFreedom)
	{
		float ExcessYaw = YawDelta > 0 
			? YawDelta - YawFreedom 
			: YawDelta + YawFreedom;
		YawCorrection = ExcessYaw * SoftLockStrength * CameraInterpSpeed * DeltaTime;
	}

	// 只修改 Yaw，保持 Pitch 和 Roll 完全不变
	FRotator NewRotation = CurrentRotation;
	NewRotation.Yaw += YawCorrection;
	// 注意：Pitch 完全不修改！玩家可以自由上下看

	OwnerController->SetControlRotation(NewRotation);
}

void UTargetingComponent::ValidateCurrentTarget(float DeltaTime)
{
	if (!LockedTarget)
	{
		ClearTarget();
		return;
	}

	// 检查目标是否仍然有效
	if (!IsTargetValid(LockedTarget))
	{
		UE_LOG(LogTemp, Log, TEXT("[TargetingComponent] Target invalid, clearing"));
		ClearTarget();
		return;
	}

	// 检查视线遮挡
	if (bCheckLineOfSight)
	{
		if (!HasLineOfSight(LockedTarget))
		{
			LineOfSightLostTimer += DeltaTime;
			
			if (LineOfSightLostTimer >= LineOfSightLostTolerance)
			{
				UE_LOG(LogTemp, Log, TEXT("[TargetingComponent] Lost line of sight, clearing target"));
				ClearTarget();
				return;
			}
		}
		else
		{
			// 视线恢复，重置计时器
			LineOfSightLostTimer = 0.0f;
		}
	}
}

float UTargetingComponent::CalculateTargetScore(AActor* Target, bool bPreferRight) const
{
	if (!Target || !GetOwner() || !OwnerController)
	{
		return -FLT_MAX;
	}

	FVector OwnerLocation = GetOwner()->GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();
	
	// 获取当前摄像机朝向
	FRotator ControlRotation = OwnerController->GetControlRotation();
	FVector ForwardVector = ControlRotation.Vector().GetSafeNormal2D();
	FVector RightVector = FVector::CrossProduct(FVector::UpVector, ForwardVector).GetSafeNormal();

	// 计算目标相对于玩家的方向
	FVector ToTarget = (TargetLocation - OwnerLocation).GetSafeNormal2D();

	// 计算目标在屏幕上的左右位置
	float RightDot = FVector::DotProduct(ToTarget, RightVector);

	// 如果希望向右切换，优先选择右边的目标（RightDot > 0）
	// 如果希望向左切换，优先选择左边的目标（RightDot < 0）
	float DirectionScore = bPreferRight ? RightDot : -RightDot;

	// 距离评分（稍近的优先）
	float Distance = FVector::Dist(OwnerLocation, TargetLocation);
	float DistanceScore = (TargetingDistance - Distance) / TargetingDistance;

	// 总评分：方向权重更高
	return DirectionScore * 100.0f + DistanceScore * 20.0f;
}

void UTargetingComponent::OnTargetDeath(AActor* DeadActor)
{
	if (DeadActor == LockedTarget)
	{
		UE_LOG(LogTemp, Log, TEXT("[TargetingComponent] Target died, attempting to find new target"));
		
		// 目标死亡，尝试自动切换到下一个目标
		AActor* OldTarget = LockedTarget;
		LockedTarget = nullptr;

		// 尝试找新目标
		AActor* NewTarget = FindBestTarget();
		if (NewTarget)
		{
			LockedTarget = NewTarget;
			LineOfSightLostTimer = 0.0f;

			// 绑定新目标死亡事件
			if (UHealthComponent* NewHealth = NewTarget->FindComponentByClass<UHealthComponent>())
			{
				NewHealth->OnDeath.AddDynamic(this, &UTargetingComponent::OnTargetDeath);
			}

			OnTargetChanged.Broadcast(NewTarget);
			UE_LOG(LogTemp, Log, TEXT("[TargetingComponent] Auto-switched to: %s"), *NewTarget->GetName());
		}
		else
		{
			// 没有新目标，结束锁定
			ClearTarget();
		}
	}
}
// ========== 锁定指示器实现 ==========

void UTargetingComponent::CreateTargetIndicator()
{
	if (!bShowTargetIndicator || !LockedTarget)
	{
		return;
	}

	// 如果已有指示器，先销毁
	DestroyTargetIndicator();

	// 创建 WidgetComponent 并附加到目标身上
	TargetIndicatorWidget = NewObject<UWidgetComponent>(LockedTarget, UWidgetComponent::StaticClass(), TEXT("LockOnIndicator"));
	
	if (!TargetIndicatorWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TargetingComponent] Failed to create target indicator widget"));
		return;
	}

	// 设置为屏幕空间模式（始终面向摄像机，不受光照影响）
	TargetIndicatorWidget->SetWidgetSpace(EWidgetSpace::Screen);
	
	// 设置绘制大小
	TargetIndicatorWidget->SetDrawSize(FVector2D(IndicatorSize, IndicatorSize));
	
	// 设置相对位置（在目标头顶）
	TargetIndicatorWidget->SetRelativeLocation(FVector(0.0f, 0.0f, IndicatorHeightOffset));
	
	// 禁用碰撞
	TargetIndicatorWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// 附加到目标 Actor 的根组件
	if (USceneComponent* RootComp = LockedTarget->GetRootComponent())
	{
		TargetIndicatorWidget->AttachToComponent(
			RootComp,
			FAttachmentTransformRules::KeepRelativeTransform
		);
	}
	
	// 注册组件
	TargetIndicatorWidget->RegisterComponent();

	// 尝试加载自定义 Widget 蓝图（如果存在）
	// 路径：Content/_BlackMythGame/UI/WBP_LockOnIndicator
	UClass* WidgetClass = LoadClass<UUserWidget>(
		nullptr, 
		TEXT("/Game/_BlackMythGame/UI/WBP_LockOnIndicator.WBP_LockOnIndicator_C")
	);
	
	if (WidgetClass)
	{
		TargetIndicatorWidget->SetWidgetClass(WidgetClass);
		UE_LOG(LogTemp, Log, TEXT("[TargetingComponent] Using custom widget: WBP_LockOnIndicator"));
	}
	else
	{
		// 如果没有自定义 Widget，使用 DrawDebug 方式作为后备
		UE_LOG(LogTemp, Warning, TEXT("[TargetingComponent] WBP_LockOnIndicator not found. Create a Widget Blueprint at: Content/_BlackMythGame/UI/WBP_LockOnIndicator"));
		UE_LOG(LogTemp, Warning, TEXT("[TargetingComponent] Using debug draw as fallback"));
	}

	UE_LOG(LogTemp, Log, TEXT("[TargetingComponent] Target indicator widget created"));
}

void UTargetingComponent::DestroyTargetIndicator()
{
	if (TargetIndicatorWidget)
	{
		TargetIndicatorWidget->DestroyComponent();
		TargetIndicatorWidget = nullptr;
		UE_LOG(LogTemp, Log, TEXT("[TargetingComponent] Target indicator destroyed"));
	}
}

void UTargetingComponent::UpdateTargetIndicator()
{
	// 如果没有自定义 Widget，使用 Debug Draw 绘制圆点
	if (bShowTargetIndicator && LockedTarget && (!TargetIndicatorWidget || !TargetIndicatorWidget->GetWidget()))
	{
		FVector TargetLocation = LockedTarget->GetActorLocation();
		TargetLocation.Z += IndicatorHeightOffset;
		
		// 使用 DrawDebugPoint 绘制一个白点（每帧绘制，始终可见）
		DrawDebugPoint(
			GetWorld(),
			TargetLocation,
			IndicatorSize,
			FColor(
				FMath::RoundToInt(IndicatorColor.R * 255),
				FMath::RoundToInt(IndicatorColor.G * 255),
				FMath::RoundToInt(IndicatorColor.B * 255),
				FMath::RoundToInt(IndicatorColor.A * 255)
			),
			false,  // bPersistentLines
			-1.0f,  // LifeTime (-1 = 1 frame)
			0       // DepthPriority (0 = always on top)
		);
	}
}