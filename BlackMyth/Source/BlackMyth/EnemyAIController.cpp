#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "EnemyBase.h"
#include "BossEnemy.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	// 初始化感知组件
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	
	// 初始化视觉配置
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if (SightConfig)
	{
		SightConfig->SightRadius = 1500.0f;
		SightConfig->LoseSightRadius = 2000.0f;
		SightConfig->PeripheralVisionAngleDegrees = 60.0f;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		
		// 将视觉配置添加到感知组件
		AIPerceptionComponent->ConfigureSense(*SightConfig);
		AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
	}
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 绑定感知更新事件
	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnPerceptionUpdated);
	}

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
	
	// 可以在这里添加调试绘制或其他每帧逻辑
}

void AEnemyAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	// 获取黑板组件
	UBlackboardComponent* BlackboardComp = GetBlackboardComponent();
	if (!BlackboardComp) return;

	for (AActor* Actor : UpdatedActors)
	{
		FActorPerceptionBlueprintInfo Info;
		if (AIPerceptionComponent->GetActorsPerception(Actor, Info))
		{
			// 检查是否是视觉刺激
			for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
			{
				if (Stimulus.Type == UAISense::GetSenseID(UAISense_Sight::StaticClass()))
				{
					// 检查是否是玩家 (这里假设玩家是 ACharacter 的子类，且不是自己人)
					// 实际项目中最好使用 Tag 或 Interface 来区分阵营
					// 这里简单判断是否是玩家控制的角色
					APawn* SensedPawn = Cast<APawn>(Actor);
					if (SensedPawn && SensedPawn->IsPlayerControlled()) 
					{
						if (Stimulus.WasSuccessfullySensed())
						{
							// 看到了玩家，更新黑板
							BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);

							// 如果是 Boss，显示血条
							if (ABossEnemy* Boss = Cast<ABossEnemy>(GetPawn()))
							{
								Boss->SetBossHealthVisibility(true);
							}
						}
						else
						{
							// 丢失视野 (可选：清除目标或保留最后已知位置)
							// BlackboardComp->ClearValue(TEXT("TargetActor"));
							
							// 如果是 Boss，丢失视野后隐藏血条 (可选)
							// if (ABossEnemy* Boss = Cast<ABossEnemy>(GetPawn()))
							// {
							// 	Boss->SetBossHealthVisibility(false);
							// }
						}
					}
				}
			}
		}
	}
}
