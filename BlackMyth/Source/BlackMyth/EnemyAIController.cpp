#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "EnemyBase.h"
#include "BossEnemy.h"
#include "WukongCharacter.h"
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
		SightConfig->PeripheralVisionAngleDegrees = 180.0f; // 扩大视野角度，防止近战时玩家绕背导致丢失仇恨
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
	
	// 如果有目标，平滑旋转朝向目标
	if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
	{
		if (AActor* Target = Cast<AActor>(BlackboardComp->GetValueAsObject(TEXT("TargetActor"))))
		{
			// 检查目标是否已经死亡
			if (AWukongCharacter* WukongTarget = Cast<AWukongCharacter>(Target))
			{
				if (WukongTarget->IsDead())
				{
					// 玩家已死亡，清除目标
					BlackboardComp->ClearValue(TEXT("TargetActor"));
					BlackboardComp->SetValueAsBool(TEXT("IsInvestigating"), false);
					
					if (AEnemyBase* Enemy = Cast<AEnemyBase>(GetPawn()))
					{
						Enemy->StartPatrolling();
					}
					UE_LOG(LogTemp, Log, TEXT("Tick: Target is dead, clearing aggro"));
					return;
				}
			}

			if (APawn* ControlledPawn = GetPawn())
			{
				// 修复：如果控制的 Pawn 已经死亡或眩晕，不再执行旋转逻辑
				if (AEnemyBase* Enemy = Cast<AEnemyBase>(ControlledPawn))
				{
					if (Enemy->IsDead() || Enemy->IsStunned()) return;
				}

				FVector Direction = Target->GetActorLocation() - ControlledPawn->GetActorLocation();
				Direction.Z = 0.0f; // 忽略高度差
				
				if (!Direction.IsNearlyZero())
				{
					FRotator TargetRotation = Direction.Rotation();
					// 使用 RInterpTo 进行平滑插值，速度设为 5.0f (可调整)
					FRotator NewRotation = FMath::RInterpTo(ControlledPawn->GetActorRotation(), TargetRotation, DeltaTime, 5.0f);
					ControlledPawn->SetActorRotation(NewRotation);
				}
			}
		}
	}
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
						// 检查玩家是否已经死亡
						AWukongCharacter* WukongChar = Cast<AWukongCharacter>(SensedPawn);
						if (WukongChar && WukongChar->IsDead())
						{
							// 玩家已死亡，清除目标并停止追击
							BlackboardComp->ClearValue(TEXT("TargetActor"));
							BlackboardComp->SetValueAsBool(TEXT("IsInvestigating"), false);
							GetWorldTimerManager().ClearTimer(LoseAggroTimer);
							
							if (AEnemyBase* Enemy = Cast<AEnemyBase>(GetPawn()))
							{
								Enemy->StartPatrolling();
							}
							UE_LOG(LogTemp, Log, TEXT("OnPerceptionUpdated: Target is dead, clearing aggro"));
							continue;
						}

						if (Stimulus.WasSuccessfullySensed())
						{
							// 看到了玩家，清除丢失仇恨的计时器
							GetWorldTimerManager().ClearTimer(LoseAggroTimer);

							// 更新黑板
							BlackboardComp->SetValueAsObject(TEXT("TargetActor"), Actor);

							// 通知 EnemyBase 播放发现动画
							if (AEnemyBase* Enemy = Cast<AEnemyBase>(GetPawn()))
							{
								Enemy->OnTargetSensed(Actor);
							}

							// 如果是 Boss，显示血条
							if (ABossEnemy* Boss = Cast<ABossEnemy>(GetPawn()))
							{
								Boss->SetBossHealthVisibility(true);
							}
						}
						else
						{
							// 丢失视野：记录最后位置，进入搜寻模式
							// 修复：使用 Stimulus.StimulusLocation 获取最后一次被感知的位置，而不是 Actor 的当前位置
							BlackboardComp->SetValueAsVector(TEXT("TargetLocation"), Stimulus.StimulusLocation);
							BlackboardComp->SetValueAsBool(TEXT("IsInvestigating"), true);
							
							// [Fix] 不要立即清除 TargetActor，否则怪会立刻转头走人
							// BlackboardComp->ClearValue(TEXT("TargetActor"));
							
							// 启动丢失仇恨计时器 (例如 5秒后彻底放弃)
							// 只有当计时器触发时，才真正清除 TargetActor
							GetWorldTimerManager().SetTimer(LoseAggroTimer, this, &AEnemyAIController::HandleLostAggro, 5.0f, false);
						}
					}
				}
			}
		}
	}
}

void AEnemyAIController::HandleLostAggro()
{
	// 真正丢失仇恨：清除黑板上的目标
	if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
	{
		BlackboardComp->ClearValue(TEXT("TargetActor"));
	}

	if (AEnemyBase* Enemy = Cast<AEnemyBase>(GetPawn()))
	{
		// 调用 EnemyBase 的重置逻辑
		Enemy->StartPatrolling();
		UE_LOG(LogTemp, Warning, TEXT("AEnemyAIController::HandleLostAggro - Lost aggro (Timer Expired), returning to patrol."));
	}
}


