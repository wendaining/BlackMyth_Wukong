#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_ClearInvestigation.generated.h"

/**
 * 行为树任务：清除搜寻状态
 */
UCLASS()
class BLACKMYTH_API UBTT_ClearInvestigation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_ClearInvestigation();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
