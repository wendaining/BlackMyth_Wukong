#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTT_RangedMoveTo.generated.h"

/**
 * 远程移动任务
 * 继承自标准 MoveTo，但增加了“视野检查”和“射程检查”
 * 如果在移动过程中：
 * 1. 距离目标小于 AcceptableRadius
 * 2. 并且 能看到目标 (LineOfSight)
 * 则提前结束任务 (Succeeded)，以便 AI 可以开始攻击
 */
UCLASS()
class BLACKMYTH_API UBTT_RangedMoveTo : public UBTTask_MoveTo
{
	GENERATED_BODY()

public:
	UBTT_RangedMoveTo();

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
