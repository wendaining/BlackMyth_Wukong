// 停用Hitbox动画通知实现

#include "AnimNotify_DeactivateHitbox.h"
#include "HitboxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UAnimNotify_DeactivateHitbox::UAnimNotify_DeactivateHitbox()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Green;
#endif
}

void UAnimNotify_DeactivateHitbox::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();

	// 查找 HitboxComponent
	TArray<UHitboxComponent*> Hitboxes;
	Owner->GetComponents<UHitboxComponent>(Hitboxes);

	for (UHitboxComponent* Hitbox : Hitboxes)
	{
		// 如果指定了名称，只停用匹配的
		if (!HitboxComponentName.IsNone() && Hitbox->GetFName() != HitboxComponentName)
		{
			continue;
		}

		// 停用
		Hitbox->DeactivateHitbox();

		UE_LOG(LogTemp, Log, TEXT("[AnimNotify] Deactivated Hitbox: %s on %s"), *Hitbox->GetName(), *Owner->GetName());
	}
}
