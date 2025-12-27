#include "XiaoTian.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "WukongCharacter.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AXiaoTian::AXiaoTian()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	CollisionBox->SetCollisionProfileName(TEXT("BlockAll")); // 改为阻止所有，确保扫到墙壁
	CollisionBox->SetBoxExtent(FVector(50, 30, 30));

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("DogMesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AXiaoTian::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AXiaoTian::OnOverlapBegin);

	// 确保忽略召唤者（防止飞出门前撞到二郎神自己）
	if (GetOwner())
	{
		CollisionBox->IgnoreActorWhenMoving(GetOwner(), true);
	}

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

	// 调试绘制碰撞盒 (红色) - [Fix] Removed debug drawing
	// if (CollisionBox)
	// {
	// 	DrawDebugBox(GetWorld(), GetActorLocation(), CollisionBox->GetUnscaledBoxExtent(), GetActorRotation().Quaternion(), FColor::Red, false, -1.0f, 0, 2.0f);
	// }

	if (CurrentState == EXiaoTianState::Flying)
	{
		// 匀速直线运动
		// [Fix] 恢复 bSweep = true 从而尊重墙壁，但我们要确保不撞到二郎神
		FVector NewLocation = GetActorLocation() + (TargetDirection * FlightSpeed * DeltaTime);
		
		FHitResult SweepHit;
		SetActorLocation(NewLocation, true, &SweepHit); 

		// [Fix] 统一碰撞逻辑：无论是撞到墙还是撞到人，统统触发“咬/撞”判定
		if (SweepHit.bBlockingHit)
		{
			AActor* HitActor = SweepHit.GetActor();
			
			// 只有当撞到的不是二郎神时，才触发咬判定（防止刷新时由于碰撞二郎神而直接消失）
			if (HitActor != GetOwner())
			{
				UE_LOG(LogTemp, Warning, TEXT("[XiaoTian] Blocking Hit: %s. Triggering Bite."), HitActor ? *HitActor->GetName() : TEXT("None"));
				TriggerBite(HitActor);
				return;
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
	// 防止重复触发
	if (CurrentState == EXiaoTianState::Biting || CurrentState == EXiaoTianState::Finished) return;

	CurrentState = EXiaoTianState::Biting;

	// 1. 造成伤害 (只有撞到悟空才扣血)
	if (Target && Target->IsA(AWukongCharacter::StaticClass()))
	{
		UGameplayStatics::ApplyDamage(Target, Damage, GetInstigatorController(), this, UDamageType::StaticClass());
		UE_LOG(LogTemp, Warning, TEXT("[XiaoTian] Hit Wukong! Applying Damage: %.1f"), Damage);
	}

	// 2. 播放咬(End)蒙太奇 (无论是撞墙还是撞人，都要播放动作展示存在感)
	if (BiteEndMontage && Mesh && Mesh->GetAnimInstance())
	{
		float Duration = Mesh->GetAnimInstance()->Montage_Play(BiteEndMontage);
		
		// 动画播完后再消失
		GetWorldTimerManager().SetTimer(LifeTimer, this, &AXiaoTian::FinishAndDestroy, FMath::Max(0.5f, Duration), false);
	}
	else
	{
		FinishAndDestroy();
	}
}

void AXiaoTian::FinishAndDestroy()
{
	if (CurrentState == EXiaoTianState::Finished) return;

	// 1. 播放消失特效 (VFX)
	if (VanishFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), VanishFX, GetActorLocation(), GetActorRotation());
	}

	CurrentState = EXiaoTianState::Finished;

	// [New] 镜头保护：如果当前镜头正对着狗，且狗要消失了，平滑切换回 Boss
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC && PC->GetViewTarget() == this && GetOwner())
	{
		// 开始向 Boss 平滑过渡
		PC->SetViewTargetWithBlend(GetOwner(), 1.0f);
		
		// 隐藏自己但暂时不销毁，保证 Camera 能够完成混合
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		
		FTimerHandle DestroyTimer;
		GetWorldTimerManager().SetTimer(DestroyTimer, this, &AXiaoTian::DestroyActor, 1.1f, false);
	}
	else
	{
		Destroy();
	}
}

void AXiaoTian::DestroyActor()
{
	Destroy();
}

void AXiaoTian::PlayEndAndVanish()
{
	UE_LOG(LogTemp, Warning, TEXT("[XiaoTian] PlayEndAndVanish called (Cinematic Mode)"));
	
	// 1. 先播放“起跳/起始”动作
	if (PounceStartMontage && Mesh && Mesh->GetAnimInstance())
	{
		float Duration = Mesh->GetAnimInstance()->Montage_Play(PounceStartMontage);
		
		// 动作播完后再执行咬(End)和后续流程
		FTimerHandle TransitionTimer;
		GetWorldTimerManager().SetTimer(TransitionTimer, [this]()
		{
			this->TriggerBite(nullptr);
		}, FMath::Max(0.5f, Duration), false);
	}
	else
	{
		// 如果没配置起始动作，直接跳到结尾
		TriggerBite(nullptr);
	}
}
