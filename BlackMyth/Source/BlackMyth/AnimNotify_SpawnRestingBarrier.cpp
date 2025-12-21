// 安息术动画通知实现

#include "AnimNotify_SpawnRestingBarrier.h"
#include "WukongCharacter.h"
#include "RestingBarrier.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

void UAnimNotify_SpawnRestingBarrier::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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

	// 检查是否设置了屏障类
	TSubclassOf<ARestingBarrier> BarrierClass = Wukong->GetRestingBarrierClass();
	if (!BarrierClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AnimNotify_SpawnRestingBarrier] RestingBarrierClass not set!"));
		return;
	}

	// 在玩家脚下生成屏障
	FVector SpawnLocation = Wukong->GetActorLocation();
	// 稍微下移到地面
	SpawnLocation.Z -= Wukong->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Wukong;
	SpawnParams.Instigator = Wukong->GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ARestingBarrier* Barrier = Wukong->GetWorld()->SpawnActor<ARestingBarrier>(
		BarrierClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (Barrier)
	{
		Barrier->InitializeBarrier(Wukong->GetRestingBarrierDuration());
		UE_LOG(LogTemp, Log, TEXT("[AnimNotify_SpawnRestingBarrier] Spawned barrier at %s"), *SpawnLocation.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[AnimNotify_SpawnRestingBarrier] Failed to spawn barrier"));
	}
}
