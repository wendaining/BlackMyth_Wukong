// 状态效果基类实现

#include "StatusEffectBase.h"
#include "../Components/StatusEffectComponent.h"

void UStatusEffectBase::Initialize(AActor* InOwner, AActor* InInstigator, float InDuration)
{
	OwnerActor = InOwner;
	Instigator = InInstigator;
	Duration = InDuration;
	RemainingTime = InDuration;

	UE_LOG(LogTemp, Log, TEXT("[StatusEffect] [%s] initialized on [%s] for %.1f seconds"),
		*StaticEnum<EStatusEffectType>()->GetNameStringByValue(static_cast<int64>(EffectType)),
		InOwner ? *InOwner->GetName() : TEXT("None"),
		InDuration);
}

void UStatusEffectBase::OnApplied_Implementation()
{
	// 基类默认实现：记录日志
	UE_LOG(LogTemp, Log, TEXT("[StatusEffect] [%s] applied to [%s]"),
		*StaticEnum<EStatusEffectType>()->GetNameStringByValue(static_cast<int64>(EffectType)),
		OwnerActor.IsValid() ? *OwnerActor->GetName() : TEXT("None"));
}

void UStatusEffectBase::OnTick_Implementation(float DeltaTime)
{
	// 基类默认实现：减少剩余时间
	RemainingTime -= DeltaTime;

	// 确保不会变成负数
	if (RemainingTime < 0.0f)
	{
		RemainingTime = 0.0f;
	}
}

void UStatusEffectBase::OnRemoved_Implementation()
{
	// 基类默认实现：记录日志
	UE_LOG(LogTemp, Log, TEXT("StatusEffect [%s] removed from [%s]"),
		*StaticEnum<EStatusEffectType>()->GetNameStringByValue(static_cast<int64>(EffectType)),
		OwnerActor.IsValid() ? *OwnerActor->GetName() : TEXT("None"));
}

void UStatusEffectBase::RefreshDuration(float NewDuration)
{
	// 刷新持续时间（用于重复施加同类型效果时）
	Duration = NewDuration;
	RemainingTime = NewDuration;

	UE_LOG(LogTemp, Log, TEXT("StatusEffect [%s] duration refreshed to %.1f seconds"),
		*StaticEnum<EStatusEffectType>()->GetNameStringByValue(static_cast<int64>(EffectType)),
		NewDuration);
}

void UStatusEffectBase::SetOwnerComponent(UStatusEffectComponent* InComponent)
{
	OwnerComponent = InComponent;
}
