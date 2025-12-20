// 销毁药瓶动画通知实现

#include "AnimNotify_DestroyPotion.h"
#include "../Items/PotionActor.h"
#include "../WukongCharacter.h"
#include "Components/SkeletalMeshComponent.h"

UAnimNotify_DestroyPotion::UAnimNotify_DestroyPotion()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Orange;
#endif
}

void UAnimNotify_DestroyPotion::Notify(USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	// 获取悟空角色
	AWukongCharacter* Wukong = Cast<AWukongCharacter>(MeshComp->GetOwner());
	if (!Wukong)
	{
		return;
	}

	// 获取并销毁药瓶
	APotionActor* Potion = Wukong->GetCurrentPotionActor();
	if (Potion)
	{
		Potion->DetachAndDestroy();
		Wukong->SetCurrentPotionActor(nullptr);

		UE_LOG(LogTemp, Log, TEXT("[AnimNotify_DestroyPotion] Destroyed potion"));
	}
}
