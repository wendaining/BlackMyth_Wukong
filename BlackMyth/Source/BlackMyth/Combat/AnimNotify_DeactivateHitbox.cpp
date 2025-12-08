// 停用Hitbox动画通知实现

#include "AnimNotify_DeactivateHitbox.h"
#include "TraceHitboxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UAnimNotify_DeactivateHitbox::UAnimNotify_DeactivateHitbox()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Green;
#endif
}

void UAnimNotify_DeactivateHitbox::Notify(USkeletalMeshComponent* MeshComp, 
	UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	int DeactivatedCount = 0;

	// 查找 HitboxComponent
	TArray<UTraceHitboxComponent*> TraceHitboxes;
	Owner->GetComponents<UTraceHitboxComponent>(TraceHitboxes);

	for (UTraceHitboxComponent* TraceHitbox : TraceHitboxes)
	{
		// 如果指定了名称，只停用匹配的
		if (!TraceHitboxComponentName.IsNone() && TraceHitbox->GetFName() != TraceHitboxComponentName)
		{
			continue;
		}

		// 停用
		TraceHitbox->DeactivateTrace();
		DeactivatedCount++;

		UE_LOG(LogTemp, Log, TEXT("[AnimNotify] Deactivated Hitbox: %s on %s"), 
			*TraceHitbox->GetName(), *Owner->GetName());
	}

	if (DeactivatedCount == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AnimNotify] No TraceHitboxComponent found to deactivate on %s"), *Owner->GetName());
	}
}
