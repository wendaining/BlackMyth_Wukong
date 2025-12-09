// Hitbox组件实现

#include "HitboxComponent.h"
#include "../Components/HealthComponent.h"
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

	// 设置默认尺寸（棍棒形状）- 移到 BeginPlay 避免构造函数问题
	SetBoxExtent(FVector(50.0f, 10.0f, 10.0f));
	
	// 不可见
	SetVisibility(false);
	SetHiddenInGame(true);

	// 设置伤害来源
	DamageInfo.Instigator = GetOwner();
	DamageInfo.DamageCauser = GetOwner();

	// 绑定碰撞事件
	OnComponentBeginOverlap.AddDynamic(this, &UHitboxComponent::OnHitboxBeginOverlap);

	// 确保初始状态是禁用的
	DeactivateHitbox();
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

void UHitboxComponent::ActivateHitbox()
{
	if (bIsActive)
	{
		return;
	}

	bIsActive = true;
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitActors.Empty();  // 清除上次的命中记录

	UE_LOG(LogTemp, Log, TEXT("[Hitbox] %s Activated"), *GetOwner()->GetName());

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

void UHitboxComponent::OnHitboxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 不激活时不处理
	if (!bIsActive)
	{
		return;
	}

	// 忽略自己
	if (bIgnoreOwner && OtherActor == GetOwner())
	{
		return;
	}

	// 忽略已命中的
	if (HitActors.Contains(OtherActor))
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

	// 广播命中事件
	OnHitDetected.Broadcast(OtherActor, HitResult);

	// 自动应用伤害
	if (bAutoApplyDamage)
	{
		ApplyDamageToTarget(OtherActor, HitResult);
	}
}

void UHitboxComponent::ApplyDamageToTarget(AActor* Target, const FHitResult& HitResult)
{
	if (!Target)
	{
		return;
	}

	// 查找目标的 HealthComponent
	UHealthComponent* TargetHealth = Target->FindComponentByClass<UHealthComponent>();
	if (TargetHealth)
	{
		// 更新伤害信息
		FDamageInfo FinalDamage = DamageInfo;
		FinalDamage.HitLocation = HitResult.ImpactPoint;
		FinalDamage.HitDirection = HitResult.ImpactNormal * -1.0f;  // 受击方向

		// 应用伤害
		float ActualDamage = FinalDamage.GetFinalDamage();
		TargetHealth->TakeDamage(ActualDamage, GetOwner());

		UE_LOG(LogTemp, Warning, TEXT("[Hitbox] Applied %.1f damage to %s (Health: %.1f)"), 
			ActualDamage, *Target->GetName(), TargetHealth->GetCurrentHealth());
	}
	else
	{
		// 目标没有 HealthComponent，使用 UE 内置伤害系统
		UGameplayStatics::ApplyDamage(
			Target,
			DamageInfo.GetFinalDamage(),
			GetOwner()->GetInstigatorController(),
			GetOwner(),
			nullptr
		);

		UE_LOG(LogTemp, Log, TEXT("[Hitbox] Applied UE damage to %s (no HealthComponent)"), *Target->GetName());
	}
}

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
