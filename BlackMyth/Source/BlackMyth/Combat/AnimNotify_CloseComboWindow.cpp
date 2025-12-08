// 关闭连击窗口动画通知实现

#include "AnimNotify_CloseComboWindow.h"
#include "../Components/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UAnimNotify_CloseComboWindow::UAnimNotify_CloseComboWindow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Orange;  // 橙色，区分于 Open 的青色
#endif
}

void UAnimNotify_CloseComboWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();

	// 查找 CombatComponent
	UCombatComponent* CombatComp = Owner->FindComponentByClass<UCombatComponent>();
	if (!CombatComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AnimNotify] No CombatComponent found on %s"), *Owner->GetName());
		return;
	}

	// 关闭连击窗口
	CombatComp->CloseComboWindow();

	// 如果配置了关闭时重置连击
	if (bResetComboOnClose)
	{
		CombatComp->ResetCombo();
		UE_LOG(LogTemp, Log, TEXT("[AnimNotify] Combo Window Closed and Reset on %s"), *Owner->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("[AnimNotify] Combo Window Closed on %s"), *Owner->GetName());
	}
}
