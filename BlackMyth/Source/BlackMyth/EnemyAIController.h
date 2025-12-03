#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 敌人 AI 控制器
 * 负责控制敌人的感知、移动和攻击逻辑
 */
UCLASS()
class BLACKMYTH_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AEnemyAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

public:
	virtual void Tick(float DeltaTime) override;

protected:
	// 感知范围
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float SightRadius = 1500.0f;

	// 攻击范围
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AttackRange = 150.0f;

	// 目标引用
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<AActor> TargetActor;
};
