#include "BTT_ClearInvestigation.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTT_ClearInvestigation::UBTT_ClearInvestigation()
{
	NodeName = "Clear Investigation";
}

EBTNodeResult::Type UBTT_ClearInvestigation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (BlackboardComp)
	{
		// 清除 "IsInvestigating" 标记
		BlackboardComp->SetValueAsBool(TEXT("IsInvestigating"), false);
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}
