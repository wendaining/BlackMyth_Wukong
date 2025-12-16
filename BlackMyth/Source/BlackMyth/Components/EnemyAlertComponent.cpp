#include "EnemyAlertComponent.h"
#include "../EnemyBase.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Engine/OverlapResult.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"

UEnemyAlertComponent::UEnemyAlertComponent()
{
	// 创建警戒图标 Widget 组件
	PrimaryComponentTick.bCanEverTick = false;
	bIsAlerted = false;
	CurrentTarget = nullptr;
}

void UEnemyAlertComponent::BeginPlay()
{
	Super::BeginPlay();

	// 缓存敌人引用
	OwnerEnemy = Cast<AEnemyBase>(GetOwner());
	if (!OwnerEnemy)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] EnemyAlertComponent: Owner is not an EnemyBase!"), *GetOwner()->GetName());
		return;
	}

	// 动态创建警戒图标 Widget 组件
	if (AlertIconWidgetClass)
	{
		AlertIconWidget = NewObject<UWidgetComponent>(OwnerEnemy, UWidgetComponent::StaticClass(), TEXT("AlertIconWidget"));
		if (AlertIconWidget)
		{
			AlertIconWidget->RegisterComponent();
			AlertIconWidget->AttachToComponent(OwnerEnemy->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			AlertIconWidget->SetRelativeLocation(FVector(0.0f, 0.0f, AlertIconHeightOffset));
			AlertIconWidget->SetWidgetSpace(EWidgetSpace::Screen); // 屏幕空间，始终面向摄像机
			AlertIconWidget->SetDrawSize(FVector2D(64.0f, 64.0f)); // 警戒图标大小
			AlertIconWidget->SetWidgetClass(AlertIconWidgetClass);
			AlertIconWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			AlertIconWidget->SetVisibility(false); // 默认隐藏

			UE_LOG(LogTemp, Log, TEXT("[%s] AlertIconWidget created and attached"), *OwnerEnemy->GetName());
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] AlertIconWidgetClass not set!"), *OwnerEnemy->GetName());
	}
}

void UEnemyAlertComponent::BroadcastAlert(AActor* Target, float AlertRadius)
{
	if (!OwnerEnemy || !Target)
	{
		return;
	}

	// 使用球形重叠检测找到范围内的所有敌人
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerEnemy);

	const FVector OwnerLocation = OwnerEnemy->GetActorLocation();
	const bool bHasOverlaps = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		OwnerLocation,
		FQuat::Identity,
		ECC_Pawn, // 只检测 Pawn
		FCollisionShape::MakeSphere(AlertRadius),
		QueryParams
	);

	if (bHasOverlaps)
	{
		int32 AlertCount = 0;
		for (const FOverlapResult& Result : OverlapResults)
		{
			if (AEnemyBase* NearbyEnemy = Cast<AEnemyBase>(Result.GetActor()))
			{
				// 确保附近的敌人没有死亡
				if (NearbyEnemy->IsDead())
				{
					continue;
				}

				// 通知附近的敌人
				if (UEnemyAlertComponent* NearbyAlertComp = NearbyEnemy->FindComponentByClass<UEnemyAlertComponent>())
				{
					NearbyAlertComp->ReceiveAlert(Target);
					AlertCount++;
				}
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("[%s] Broadcast alert to %d nearby enemies (Radius: %.1f)"),
			*OwnerEnemy->GetName(), AlertCount, AlertRadius);
	}

	// 播放警报音效
	if (AlertSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AlertSound, OwnerLocation);
	}
}

void UEnemyAlertComponent::ReceiveAlert(AActor* AlertTarget)
{
	if (!OwnerEnemy || !AlertTarget)
	{
		return;
	}

	// 如果已经在警戒状态，不重复处理
	if (bIsAlerted && CurrentTarget == AlertTarget)
	{
		return;
	}

	// 如果敌人已经死亡，不处理警报
	if (OwnerEnemy->IsDead())
	{
		return;
	}

	// 设置警戒状态
	bIsAlerted = true;
	CurrentTarget = AlertTarget;

	UE_LOG(LogTemp, Warning, TEXT("[%s] Received alert! Target: %s"),
		*OwnerEnemy->GetName(), *AlertTarget->GetName());

	// 显示警戒图标
	ShowAlertIcon(true);

	// 通知 EnemyBase 设置战斗目标，让敌人进入战斗状态 
	OwnerEnemy->OnTargetSensed(AlertTarget);

	// 如果启用了自动隐藏，设置计时器
	if (bAutoHideIcon && AutoHideDelay > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			AutoHideTimer,
			this,
			&UEnemyAlertComponent::AutoHideAlertIcon,
			AutoHideDelay,
			false
		);
	}
}

void UEnemyAlertComponent::ShowAlertIcon(bool bShow)
{
	if (AlertIconWidget)
	{
		AlertIconWidget->SetVisibility(bShow);
		UE_LOG(LogTemp, Log, TEXT("[%s] Alert icon %s"),
			*OwnerEnemy->GetName(), bShow ? TEXT("shown") : TEXT("hidden"));
	}
}

void UEnemyAlertComponent::AutoHideAlertIcon()
{
	ShowAlertIcon(false);
	bIsAlerted = false;
}
