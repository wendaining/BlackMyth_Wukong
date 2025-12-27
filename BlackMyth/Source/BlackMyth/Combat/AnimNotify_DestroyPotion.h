// 销毁药瓶动画通知

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_DestroyPotion.generated.h"

/**
 * 销毁药瓶动画通知
 * 在喝药动画结束时添加，销毁药瓶
 */
UCLASS(meta = (DisplayName = "Destroy Potion"))
class BLACKMYTH_API UAnimNotify_DestroyPotion : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_DestroyPotion();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("Destroy Potion");
	}
};
