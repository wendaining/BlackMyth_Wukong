#include "XiaoTian.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "WukongCharacter.h"
#include "DrawDebugHelpers.h"

AXiaoTian::AXiaoTian()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	CollisionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionBox->SetBoxExtent(FVector(50, 30, 30));

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DogMesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AXiaoTian::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AXiaoTian::OnOverlapBegin);

	// 1. 寻找玩家作为目标
	if (AActor* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		TargetDirection = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		TargetDirection.Z = 0; // 水平飞行
		
		// 转向玩家
		SetActorRotation(TargetDirection.Rotation());
	}

	// [New] 稍微抬高一点点，防止狗肚子蹭地
	AddActorWorldOffset(FVector(0, 0, 20.0f));

	SpawnLocation = GetActorLocation();

	// 2. 播放启动动作
	if (PounceStartMontage && Mesh && Mesh->GetAnimInstance())
	{
		Mesh->GetAnimInstance()->Montage_Play(PounceStartMontage);
		
		// 使用可调的 LaunchDelay 进行起飞判定
		FTimerHandle StartTimer;
		GetWorldTimerManager().SetTimer(StartTimer, this, &AXiaoTian::StartFlying, LaunchDelay, false);
	}
	else
	{
		StartFlying();
	}

	// 增加飞行距离限制（从 1500 提高到 5000，确保能飞跃整个竞技场）
	MaxFlightDistance = 5000.0f;
	
	// 保底销毁 (5秒，给足时间飞)
	GetWorldTimerManager().SetTimer(LifeTimer, this, &AXiaoTian::FinishAndDestroy, 5.0f, false);
}

void AXiaoTian::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 调试绘制碰撞盒 (红色)
	if (CollisionBox)
	{
		DrawDebugBox(GetWorld(), GetActorLocation(), CollisionBox->GetUnscaledBoxExtent(), GetActorRotation().Quaternion(), FColor::Red, false, -1.0f, 0, 2.0f);
	}

	if (CurrentState == EXiaoTianState::Flying)
	{
		// 匀速直线运动
		// [Fix] 恢复 bSweep = true 从而尊重墙壁，但我们要确保不撞到二郎神
		FVector NewLocation = GetActorLocation() + (TargetDirection * FlightSpeed * DeltaTime);
		
		FHitResult SweepHit;
		SetActorLocation(NewLocation, true, &SweepHit); 

		// 如果扫到了墙壁（StaticMesh），则消失
		if (SweepHit.IsValidBlockingHit())
		{
			if (AActor* HitActor = SweepHit.GetActor())
			{
				// 撞到场景或障碍物
				if (!HitActor->IsA(ACharacter::StaticClass()))
				{
					UE_LOG(LogTemp, Warning, TEXT("[XiaoTian] Hit Obstacle: %s. Destroying."), *HitActor->GetName());
					FinishAndDestroy();
					return;
				}
			}
		}

		// 距离检查
		if (FVector::Dist(SpawnLocation, NewLocation) > MaxFlightDistance)
		{
			UE_LOG(LogTemp, Warning, TEXT("[XiaoTian] Reached Max Distance. Vanishing."));
			FinishAndDestroy();
		}
	}
}

void AXiaoTian::StartFlying()
{
	if (CurrentState == EXiaoTianState::Spawning)
	{
		CurrentState = EXiaoTianState::Flying;
		
		// 循环播放飞行(Loop)动画
		if (PounceLoopMontage && Mesh && Mesh->GetAnimInstance())
		{
			Mesh->GetAnimInstance()->Montage_Play(PounceLoopMontage);
		}

		// [Fix] 检查是否已经重叠（如果生成时就在玩家身上，BeginOverlap 不会触发）
		TArray<AActor*> OverlappingActors;
		GetOverlappingActors(OverlappingActors, AWukongCharacter::StaticClass());
		for (AActor* Actor : OverlappingActors)
		{
			if (Actor != GetOwner())
			{
				TriggerBite(Actor);
				break;
			}
		}
	}
}

void AXiaoTian::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 确保只撞击玩家 - 直接转型检查，更可靠
	if (OtherActor && OtherActor != GetOwner() && OtherActor->IsA(AWukongCharacter::StaticClass()))
	{
		if (CurrentState == EXiaoTianState::Flying)
		{
			TriggerBite(OtherActor);
		}
	}
}

void AXiaoTian::TriggerBite(AActor* Target)
{
	CurrentState = EXiaoTianState::Biting;

	// 1. 造成伤害
	UGameplayStatics::ApplyDamage(Target, Damage, GetInstigatorController(), this, nullptr);

	// 2. 播放咬(End)蒙太奇
	if (BiteEndMontage && Mesh && Mesh->GetAnimInstance())
	{
		float Duration = Mesh->GetAnimInstance()->Montage_Play(BiteEndMontage);
		
		// 动画播完后再消失
		GetWorldTimerManager().SetTimer(LifeTimer, this, &AXiaoTian::FinishAndDestroy, Duration, false);
	}
	else
	{
		FinishAndDestroy();
	}

	UE_LOG(LogTemp, Warning, TEXT("[XiaoTian] Bit the target!"));
}

void AXiaoTian::FinishAndDestroy()
{
	CurrentState = EXiaoTianState::Finished;
	Destroy();
}
