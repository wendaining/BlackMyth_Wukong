#include "RegularEnemy.h"
#include "BehaviorTree/BehaviorTree.h"

ARegularEnemy::ARegularEnemy()
{
	// 强制设置近战攻击距离，防止被其他逻辑污染或蓝图误设
	AttackRadius = 150.0f; 
	
	// 确保移动速度适合近战追击
	ChasingSpeed = 350.0f;
}

void ARegularEnemy::BeginPlay()
{
	Super::BeginPlay();

	// 检查行为树是否错误地分配成了远程行为树
	if (BehaviorTree)
	{
		FString BTName = BehaviorTree->GetName();
		UE_LOG(LogTemp, Warning, TEXT("[%s] Using Behavior Tree: %s"), *GetName(), *BTName);

		if (BTName.Contains(TEXT("Ranged")))
		{
			UE_LOG(LogTemp, Error, TEXT("!!! CONFIGURATION ERROR !!! [%s] is a RegularEnemy (Melee) but is using a RANGED Behavior Tree (%s)! This will cause it to run away or stop too far. Please change the Behavior Tree in the Blueprint Class Defaults to 'BT_Enemy'."), *GetName(), *BTName);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] No Behavior Tree assigned!"), *GetName());
	}
}
