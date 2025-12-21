// 安息术动画通知 - 在画圈动画播放到关键帧时生成屏障

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SpawnRestingBarrier.generated.h"

/**
 * 安息术屏障生成通知
 * 在画圈动画播放到指定帧时生成屏障（确保先画圈，再生成屏障）
 */
UCLASS()
class BLACKMYTH_API UAnimNotify_SpawnRestingBarrier : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
