#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "EnemyBase.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 如果有行为树，优先运行行为树
	if (AEnemyBase* Enemy = Cast<AEnemyBase>(InPawn))
	{
		if (Enemy->GetBehaviorTree())
		{
			RunBehaviorTree(Enemy->GetBehaviorTree());
		}
	}
}

void AEnemyAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 如果正在运行行为树，则跳过简单的 Tick 逻辑
	if (BrainComponent && BrainComponent->IsRunning())
	{
		return;
	}

	// 简单的 AI 逻辑：寻找并追逐玩家
	// 注意：这是最基础的实现，后续建议使用行为树 (Behavior Tree) 替代
	if (!TargetActor)
	{
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		if (PlayerPawn)
		{
			float Distance = FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());
			if (Distance <= SightRadius)
			{
				TargetActor = PlayerPawn;
			}
		}
	}

	if (TargetActor)
	{
		// 检查目标是否死亡
		// ABlackMythCharacter* TargetChar = Cast<ABlackMythCharacter>(TargetActor);
		// if (TargetChar && TargetChar->IsDead()) { TargetActor = nullptr; StopMovement(); return; }

		float Distance = FVector::Dist(GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());
		
		if (Distance > AttackRange)
		{
			// 追逐
			MoveToActor(TargetActor, AttackRange * 0.8f); // 移动到攻击范围内
		}
		else
		{
			// 停止移动，准备攻击
			StopMovement();
			
			// 这里可以调用 EnemyBase 的攻击接口
			// if (AEnemyBase* Enemy = Cast<AEnemyBase>(GetPawn()))
			// {
			//     Enemy->Attack();
			// }
		}
	}
}
