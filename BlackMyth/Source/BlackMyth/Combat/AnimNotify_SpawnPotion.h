// 生成药瓶动画通知

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_SpawnPotion.generated.h"

class APotionActor;

/**
 * 生成药瓶动画通知
 * 在喝药动画开始时添加，生成药瓶并附加到角色手上
 */
UCLASS(meta = (DisplayName = "Spawn Potion"))
class BLACKMYTH_API UAnimNotify_SpawnPotion : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_SpawnPotion();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("Spawn Potion");
	}
};
