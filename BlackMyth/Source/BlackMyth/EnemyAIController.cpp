#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "EnemyBase.h"
#include "BossEnemy.h"
#include "WukongCharacter.h"
#include "WukongClone.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "Components/TeamComponent.h"
#include "Components/HealthComponent.h"

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
		SightConfig->PeripheralVisionAngleDegrees = 90.0f; // 改为 90 度 (前方 180 度视野)，允许背后偷袭
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
			// 检查目标是否已经死亡（玩家或分身）
			bool bTargetIsDead = false;
			
			if (AWukongCharacter* WukongTarget = Cast<AWukongCharacter>(Target))
			{
				bTargetIsDead = WukongTarget->IsDead();
			}
			else if (AWukongClone* CloneTarget = Cast<AWukongClone>(Target))
			{
				// 检查分身是否有效（可能已被销毁或生命值归零）
				if (UHealthComponent* CloneHealth = CloneTarget->FindComponentByClass<UHealthComponent>())
				{
					bTargetIsDead = CloneHealth->IsDead();
				}
			}
			
			if (bTargetIsDead)
			{
				// 目标已死亡，清除目标并尝试寻找新目标
				BlackboardComp->ClearValue(TEXT("TargetActor"));
				BlackboardComp->SetValueAsBool(TEXT("IsInvestigating"), false);
				
				// 尝试寻找新的敌对目标
				AActor* NewTarget = FindNearestHostileTarget();
				if (NewTarget)
				{
					BlackboardComp->SetValueAsObject(TEXT("TargetActor"), NewTarget);
					UE_LOG(LogTemp, Log, TEXT("Tick: Previous target dead, switching to new target"));
				}
				else
				{
					if (AEnemyBase* Enemy = Cast<AEnemyBase>(GetPawn()))
					{
						Enemy->StartPatrolling();
					}
					UE_LOG(LogTemp, Log, TEXT("Tick: Target is dead, no other targets, returning to patrol"));
				}
				return;
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

	// 获取自身的阵营组件
	UTeamComponent* MyTeamComp = nullptr;
	if (APawn* MyPawn = GetPawn())
	{
		MyTeamComp = MyPawn->FindComponentByClass<UTeamComponent>();
	}

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
					// 使用阵营组件判断是否是敌对目标
					bool bIsHostile = false;
					
					if (MyTeamComp)
					{
						// 使用阵营组件判断敌对关系
						bIsHostile = MyTeamComp->IsHostileToActor(Actor);
					}
					else
					{
						// 后备方案：检查是否是玩家控制的角色
						APawn* SensedPawn = Cast<APawn>(Actor);
						bIsHostile = SensedPawn && SensedPawn->IsPlayerControlled();
					}
					
					if (bIsHostile)
					{
						// 检查目标是否已经死亡或处于变身状态
						bool bTargetIsDead = false;
						bool bTargetIsTransformed = false;
						
						if (AWukongCharacter* WukongChar = Cast<AWukongCharacter>(Actor))
						{
							bTargetIsDead = WukongChar->IsDead();
							bTargetIsTransformed = WukongChar->IsTransformed();
						}
						else if (AWukongClone* CloneActor = Cast<AWukongClone>(Actor))
						{
							if (UHealthComponent* CloneHealth = CloneActor->FindComponentByClass<UHealthComponent>())
							{
								bTargetIsDead = CloneHealth->IsDead();
							}
						}
						
						// 如果目标处于变身状态，完全忽略
						if (bTargetIsTransformed)
						{
							UE_LOG(LogTemp, Log, TEXT("OnPerceptionUpdated: Ignoring transformed Wukong"));
							continue;
						}
						
						if (bTargetIsDead)
						{
							// 目标已死亡，清除目标并尝试找新目标
							BlackboardComp->ClearValue(TEXT("TargetActor"));
							BlackboardComp->SetValueAsBool(TEXT("IsInvestigating"), false);
							GetWorldTimerManager().ClearTimer(LoseAggroTimer);
							
							// 尝试寻找新的敌对目标
							AActor* NewTarget = FindNearestHostileTarget();
							if (NewTarget)
							{
								BlackboardComp->SetValueAsObject(TEXT("TargetActor"), NewTarget);
							}
							else if (AEnemyBase* Enemy = Cast<AEnemyBase>(GetPawn()))
							{
								Enemy->StartPatrolling();
							}
							UE_LOG(LogTemp, Log, TEXT("OnPerceptionUpdated: Target is dead, switching target or patrolling"));
							continue;
						}

						if (Stimulus.WasSuccessfullySensed())
						{
							// 看到了敌对目标，清除丢失仇恨的计时器
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
							
							UE_LOG(LogTemp, Log, TEXT("OnPerceptionUpdated: Hostile target sensed: %s"), *Actor->GetName());
						}
						else
						{
							// 丢失视野：记录最后位置，进入搜寻模式
							BlackboardComp->SetValueAsVector(TEXT("TargetLocation"), Stimulus.StimulusLocation);
							BlackboardComp->SetValueAsBool(TEXT("IsInvestigating"), true);
							
							// 启动丢失仇恨计时器 (例如 5秒后彻底放弃)
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

AActor* AEnemyAIController::FindNearestHostileTarget()
{
	APawn* MyPawn = GetPawn();
	if (!MyPawn) return nullptr;
	
	UTeamComponent* MyTeamComp = MyPawn->FindComponentByClass<UTeamComponent>();
	if (!MyTeamComp) return nullptr;
	
	AActor* NearestTarget = nullptr;
	float NearestDistSq = FLT_MAX;
	FVector MyLocation = MyPawn->GetActorLocation();
	
	// 获取所有被感知到的Actor
	TArray<AActor*> PerceivedActors;
	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);
	}
	
	for (AActor* Actor : PerceivedActors)
	{
		if (!Actor || Actor == MyPawn) continue;
		
		// 检查是否是敌对目标
		if (!MyTeamComp->IsHostileToActor(Actor)) continue;
		
		// 检查目标是否已经死亡
		bool bIsDead = false;
		if (AWukongCharacter* Wukong = Cast<AWukongCharacter>(Actor))
		{
			bIsDead = Wukong->IsDead();
		}
		else if (AWukongClone* Clone = Cast<AWukongClone>(Actor))
		{
			if (UHealthComponent* Health = Clone->FindComponentByClass<UHealthComponent>())
			{
				bIsDead = Health->IsDead();
			}
		}
		
		if (bIsDead) continue;
		
		// 计算距离
		float DistSq = FVector::DistSquared(MyLocation, Actor->GetActorLocation());
		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			NearestTarget = Actor;
		}
	}
	
	return NearestTarget;
}


