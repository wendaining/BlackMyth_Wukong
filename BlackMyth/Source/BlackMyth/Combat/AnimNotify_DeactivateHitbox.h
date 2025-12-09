// 停用TraceHitbox动画通知

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_DeactivateHitbox.generated.h"

/**
 * 停用 TraceHitbox 动画通知
 * 在攻击动画的结束帧添加，关闭伤害检测窗口
 */
UCLASS(meta = (DisplayName = "Deactivate Hitbox"))
class BLACKMYTH_API UAnimNotify_DeactivateHitbox : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_DeactivateHitbox();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, 
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override 
	{ 
		return TEXT("Deactivate Hitbox"); 
	}

public:
	/** 要停用的 TraceHitbox 组件名称（留空则停用所有） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hitbox")
	FName TraceHitboxComponentName;
};
