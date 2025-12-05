#include "BTT_FindRandomPatrol.h"
#include "EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "EnemyBase.h"

UBTT_FindRandomPatrol::UBTT_FindRandomPatrol()
{
	NodeName = "Find Random Patrol";
}

EBTNodeResult::Type UBTT_FindRandomPatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 获取控制器和 Pawn
	AEnemyAIController* AIController = Cast<AEnemyAIController>(OwnerComp.GetAIOwner());
	if (!AIController) return EBTNodeResult::Failed;

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn) return EBTNodeResult::Failed;

	// 获取导航系统
	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSystem) return EBTNodeResult::Failed;

	// 寻找随机点
	FNavLocation RandomLocation;
	FVector Origin = ControlledPawn->GetActorLocation();
	
	// 如果是 EnemyBase，可以尝试使用其定义的巡逻半径（如果有的话，这里暂时使用 Task 自己的参数）
	
	if (NavSystem->GetRandomReachablePointInRadius(Origin, PatrolRadius, RandomLocation))
	{
		// 更新黑板
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(TargetLocationKey.SelectedKeyName, RandomLocation.Location);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
