// 激活TraceTraceHitbox动画通知实现

#include "AnimNotify_ActivateHitbox.h"
#include "TraceHitboxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UAnimNotify_ActivateHitbox::UAnimNotify_ActivateHitbox()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Red;
#endif
}

void UAnimNotify_ActivateHitbox::Notify(USkeletalMeshComponent* MeshComp, 
	UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	int ActivatedCount = 0;

	// 查找 TraceHitboxComponent
	TArray<UTraceHitboxComponent*> TraceHitboxes;
	Owner->GetComponents<UTraceHitboxComponent>(TraceHitboxes);

	if (TraceHitboxes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AnimNotify] No TraceHitboxComponent found on %s"), *Owner->GetName());
		return;
	}

	for (UTraceHitboxComponent* TraceHitbox : TraceHitboxes)
	{
		// 如果指定了名称，只激活匹配的
		if (!TraceHitboxComponentName.IsNone() && TraceHitbox->GetFName() != TraceHitboxComponentName)
		{
			continue;
		}

		// 覆盖伤害
		if (OverrideDamage > 0.0f)
		{
			TraceHitbox->SetBaseDamage(OverrideDamage);
		}

		// 设置攻击类型
		TraceHitbox->SetHeavyAttack(bIsHeavyAttack);
		TraceHitbox->SetAirAttack(bIsAirAttack);

		// 激活 TraceHitbox
		TraceHitbox->ActivateTrace();
		ActivatedCount++;

		UE_LOG(LogTemp, Log, TEXT("[AnimNotify] Activated TraceHitbox: %s on %s (Heavy: %s, Air: %s)"),
			*TraceHitbox->GetName(), *Owner->GetName(),
			bIsHeavyAttack ? TEXT("YES") : TEXT("NO"),
			bIsAirAttack ? TEXT("YES") : TEXT("NO"));
	}
}
