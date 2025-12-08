// 关闭连击窗口动画通知

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_CloseComboWindow.generated.h"

/**
 * 关闭连击窗口动画通知
 * 如果玩家在窗口期间没有输入，连击将被重置
 */
UCLASS(meta = (DisplayName = "Close Combo Window"))
class BLACKMYTH_API UAnimNotify_CloseComboWindow : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_CloseComboWindow();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override { return TEXT("Close Combo Window"); }

public:
	/** 关闭窗口时是否重置连击（默认不重置，让 CombatComponent 的超时机制处理） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	bool bResetComboOnClose = false;
};
