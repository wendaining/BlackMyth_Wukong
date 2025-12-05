// 激活Hitbox动画通知实现

#include "AnimNotify_ActivateHitbox.h"
#include "HitboxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UAnimNotify_ActivateHitbox::UAnimNotify_ActivateHitbox()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Red;
#endif
}

void UAnimNotify_ActivateHitbox::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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

	if (Hitboxes.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AnimNotify] No HitboxComponent found on %s"), *Owner->GetName());
		return;
	}

	for (UHitboxComponent* Hitbox : Hitboxes)
	{
		// 如果指定了名称，只激活匹配的
		if (!HitboxComponentName.IsNone() && Hitbox->GetFName() != HitboxComponentName)
		{
			continue;
		}

		// 覆盖伤害
		if (OverrideDamage > 0.0f)
		{
			Hitbox->SetBaseDamage(OverrideDamage);
		}

		// 设置攻击类型，影响 CombatComponent 的伤害计算
		Hitbox->SetHeavyAttack(bIsHeavyAttack);
		Hitbox->SetAirAttack(bIsAirAttack);

		// 激活 Hitbox
		Hitbox->ActivateHitbox();

		UE_LOG(LogTemp, Log, TEXT("[AnimNotify] Activated Hitbox: %s on %s (Heavy: %s, Air: %s)"),
			*Hitbox->GetName(), *Owner->GetName(),
			bIsHeavyAttack ? TEXT("YES") : TEXT("NO"),
			bIsAirAttack ? TEXT("YES") : TEXT("NO"));
	}
}
