// 开启连击窗口动画通知
// 用于在动画中控制连击输入窗口

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_OpenComboWindow.generated.h"

/**
 * 开启连击窗口动画通知，允许玩家输入下一段连击
 */
UCLASS(meta = (DisplayName = "Open Combo Window"))
class BLACKMYTH_API UAnimNotify_OpenComboWindow : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_OpenComboWindow();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("Open Combo Window"); }
};
