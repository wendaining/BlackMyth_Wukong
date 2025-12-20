// 生成药瓶动画通知实现

#include "AnimNotify_SpawnPotion.h"
#include "../Items/PotionActor.h"
#include "../WukongCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UAnimNotify_SpawnPotion::UAnimNotify_SpawnPotion()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor::Green;
#endif
}

void UAnimNotify_SpawnPotion::Notify(USkeletalMeshComponent* MeshComp,
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
		UE_LOG(LogTemp, Warning, TEXT("[AnimNotify_SpawnPotion] Owner is not WukongCharacter"));
		return;
	}

	// 获取当前使用的药瓶类
	TSubclassOf<APotionActor> PotionClass = Wukong->GetCurrentPotionClass();
	if (!PotionClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AnimNotify_SpawnPotion] No PotionClass set on WukongCharacter"));
		return;
	}

	// 生成药瓶
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Wukong;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APotionActor* Potion = Wukong->GetWorld()->SpawnActor<APotionActor>(
		PotionClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (Potion)
	{
		// 附加到角色并显示
		Potion->AttachToCharacter(Wukong);
		Potion->ShowPotion();

		// 保存引用以便后续销毁
		Wukong->SetCurrentPotionActor(Potion);

		UE_LOG(LogTemp, Log, TEXT("[AnimNotify_SpawnPotion] Spawned potion: %s"), *Potion->GetName());
	}
}
