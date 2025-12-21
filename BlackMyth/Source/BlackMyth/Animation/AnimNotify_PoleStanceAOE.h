// 立棍法AOE伤害通知 - 落地瞬间触发范围伤害和破韧

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_PoleStanceAOE.generated.h"

/**
 * 立棍技能AOE伤害通知
 * 在立棍落地瞬间触发，对范围内所有敌人造成伤害并破坏韧性
 */
UCLASS()
class BLACKMYTH_API UAnimNotify_PoleStanceAOE : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_PoleStanceAOE();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	// ========== 配置参数 ==========

	/** AOE伤害半径 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	float AOERadius = 400.0f;

	/** AOE伤害值（会触发敌人正常的受击硬直） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	float Damage = 60.0f;

	/** 是否绘制调试球体 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawDebug = true;

	/** 调试球体持续时间 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug", meta = (EditCondition = "bDrawDebug"))
	float DebugDrawDuration = 2.0f;

#if WITH_EDITOR
	virtual FString GetNotifyName_Implementation() const override;
#endif
};
