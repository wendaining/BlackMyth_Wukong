#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_FindRandomPatrol.generated.h"

/**
 * 行为树任务：寻找随机巡逻点
 */
UCLASS()
class BLACKMYTH_API UBTT_FindRandomPatrol : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_FindRandomPatrol();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	/** 巡逻半径 */
	UPROPERTY(EditAnywhere, Category = "AI")
	float PatrolRadius = 1000.0f;

	/** 黑板 Key：目标位置 */
	UPROPERTY(EditAnywhere, Category = "AI")
	struct FBlackboardKeySelector TargetLocationKey;
};
