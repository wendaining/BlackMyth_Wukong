// 开启连击窗口动画通知

#include "AnimNotify_OpenComboWindow.h"
#include "../Components/CombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UAnimNotify_OpenComboWindow::UAnimNotify_OpenComboWindow()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Cyan;  // 青色，区分于 Hitbox 的红色
#endif
}

void UAnimNotify_OpenComboWindow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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

	// 开启连击窗口
	CombatComp->OpenComboWindow();

	UE_LOG(LogTemp, Log, TEXT("[AnimNotify] Combo Window Opened on %s (Combo: %d/%d)"),
		*Owner->GetName(), CombatComp->GetCurrentComboIndex() + 1, CombatComp->GetMaxComboCount());
}
