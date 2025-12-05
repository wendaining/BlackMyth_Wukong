#include "BTT_RangedMoveTo.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"

UBTT_RangedMoveTo::UBTT_RangedMoveTo()
{
	NodeName = "Ranged Move To";
	bNotifyTick = true; // 启用 Tick，以便每帧检查视野
}

void UBTT_RangedMoveTo::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	// 获取 AI 控制器
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return;

	// 获取目标 Actor
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(BlackboardKey.SelectedKeyName));

	if (TargetActor)
	{
		// 1. 检查距离
		float DistSq = FVector::DistSquared(AIController->GetPawn()->GetActorLocation(), TargetActor->GetActorLocation());
		float AcceptableRadiusSq = FMath::Square(AcceptableRadius);

		if (DistSq <= AcceptableRadiusSq)
		{
			// 2. 检查视野 (Line Of Sight)
			// LineOfSightTo 会进行射线检测，检查是否有墙壁阻挡
			if (AIController->LineOfSightTo(TargetActor))
			{
				// 满足条件：在射程内且看得到目标
				// 停止移动
				AIController->StopMovement();
				
				// 结束任务，返回成功
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			}
		}
	}
}
