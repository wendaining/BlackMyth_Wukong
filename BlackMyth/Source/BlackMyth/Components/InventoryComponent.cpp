// 背包组件实现

#include "InventoryComponent.h"
#include "../WukongCharacter.h"
#include "HealthComponent.h"
#include "StaminaComponent.h"
#include "StatusEffectComponent.h"
#include "../StatusEffect/AttackBuffEffect.h"
#include "../StatusEffect/DefenseBuffEffect.h"
#include "../StatusEffect/HealingIndicatorEffect.h"
#include "../StatusEffect/StaminaIndicatorEffect.h"

FItemSlot UInventoryComponent::EmptySlot;

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// 如果没有在编辑器中配置，创建默认物品槽
	if (ItemSlots.Num() == 0)
	{
		// 血药
		FItemSlot HealthPotionSlot;
		HealthPotionSlot.ItemType = EItemType::HealthPotion;
		HealthPotionSlot.CurrentCount = 3;
		HealthPotionSlot.MaxCount = 5;  // 允许购买更多
		HealthPotionSlot.EffectValue = 50.0f;  // 回复50点血
		HealthPotionSlot.EffectDuration = 0.0f;
		HealthPotionSlot.DisplayName = FText::FromString(TEXT("血药"));
		ItemSlots.Add(HealthPotionSlot);

		// 体力药
		FItemSlot StaminaPotionSlot;
		StaminaPotionSlot.ItemType = EItemType::StaminaPotion;
		StaminaPotionSlot.CurrentCount = 2;
		StaminaPotionSlot.MaxCount = 5;  // 允许购买更多
		StaminaPotionSlot.EffectValue = 50.0f;  // 回复50点体力
		StaminaPotionSlot.EffectDuration = 0.0f;
		StaminaPotionSlot.DisplayName = FText::FromString(TEXT("体力药"));
		ItemSlots.Add(StaminaPotionSlot);

		// 攻击Buff
		FItemSlot AttackBuffSlot;
		AttackBuffSlot.ItemType = EItemType::AttackBuff;
		AttackBuffSlot.CurrentCount = 1;
		AttackBuffSlot.MaxCount = 3;  // 允许购买更多
		AttackBuffSlot.EffectValue = 1.3f;      // 攻击力提升30%
		AttackBuffSlot.EffectDuration = 10.0f;  // 持续10秒
		AttackBuffSlot.DisplayName = FText::FromString(TEXT("怒火丹"));
		ItemSlots.Add(AttackBuffSlot);

		// 防御Buff
		FItemSlot DefenseBuffSlot;
		DefenseBuffSlot.ItemType = EItemType::DefenseBuff;
		DefenseBuffSlot.CurrentCount = 1;
		DefenseBuffSlot.MaxCount = 3;  // 允许购买更多
		DefenseBuffSlot.EffectValue = 0.5f;     // 受伤减免50%
		DefenseBuffSlot.EffectDuration = 10.0f; // 持续10秒
		DefenseBuffSlot.DisplayName = FText::FromString(TEXT("金刚丹"));
		ItemSlots.Add(DefenseBuffSlot);
	}
}

bool UInventoryComponent::UseItem(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= ItemSlots.Num())
		return false;

	FItemSlot& Slot = ItemSlots[SlotIndex];

	if (!Slot.HasItem())
		return false;

	// 消耗物品
	Slot.Consume();

	// 应用效果
	ApplyItemEffect(Slot);

	// 广播事件
	OnItemUsed.Broadcast(SlotIndex);
	OnItemCountChanged.Broadcast(SlotIndex, Slot.CurrentCount);

	return true;
}

bool UInventoryComponent::UseSelectedItem()
{
	return UseItem(SelectedSlotIndex);
}

void UInventoryComponent::SelectSlot(int32 SlotIndex)
{
	if (SlotIndex >= 0 && SlotIndex < ItemSlots.Num() && SlotIndex != SelectedSlotIndex)
	{
		SelectedSlotIndex = SlotIndex;
		OnSelectedSlotChanged.Broadcast(SelectedSlotIndex);
	}
}

void UInventoryComponent::SelectNextSlot()
{
	int32 NewIndex = (SelectedSlotIndex + 1) % ItemSlots.Num();
	SelectSlot(NewIndex);
}

void UInventoryComponent::SelectPreviousSlot()
{
	int32 NewIndex = (SelectedSlotIndex - 1 + ItemSlots.Num()) % ItemSlots.Num();
	SelectSlot(NewIndex);
}

void UInventoryComponent::RefillAllItems()
{
	for (int32 i = 0; i < ItemSlots.Num(); i++)
	{
		if (ItemSlots[i].CurrentCount != ItemSlots[i].MaxCount)
		{
			ItemSlots[i].CurrentCount = ItemSlots[i].MaxCount;
			OnItemCountChanged.Broadcast(i, ItemSlots[i].CurrentCount);
		}
	}
}

int32 UInventoryComponent::GetItemCount(EItemType Type) const
{
	for (const FItemSlot& Slot : ItemSlots)
	{
		if (Slot.ItemType == Type)
			return Slot.CurrentCount;
	}
	return 0;
}

bool UInventoryComponent::AddItemCount(EItemType Type, int32 Amount)
{
	if (Amount == 0)
	{
		return false;
	}

	for (int32 i = 0; i < ItemSlots.Num(); ++i)
	{
		FItemSlot& Slot = ItemSlots[i];
		if (Slot.ItemType == Type)
		{
			const int32 OldCount = Slot.CurrentCount;
			const int32 NewCount = FMath::Clamp(OldCount + Amount, 0, Slot.MaxCount);

			if (NewCount != OldCount)
			{
				Slot.CurrentCount = NewCount;
				OnItemCountChanged.Broadcast(i, Slot.CurrentCount);
				return true;
			}

			// 未发生变化（例如已达上限或下限）
			return false;
		}
	}

	// 未找到该类型槽位
	return false;
}

const FItemSlot& UInventoryComponent::GetItemSlot(int32 SlotIndex) const
{
	if (SlotIndex >= 0 && SlotIndex < ItemSlots.Num())
		return ItemSlots[SlotIndex];
	return EmptySlot;
}

void UInventoryComponent::ApplyItemEffect(const FItemSlot& Item)
{
	AWukongCharacter* Owner = Cast<AWukongCharacter>(GetOwner());
	if (!Owner) return;

	UStatusEffectComponent* StatusEffect = Owner->FindComponentByClass<UStatusEffectComponent>();

	switch (Item.ItemType)
	{
	case EItemType::HealthPotion:
		// 先应用治疗效果
		if (UHealthComponent* Health = Owner->FindComponentByClass<UHealthComponent>())
		{
			Health->Heal(Item.EffectValue);
		}
		// 再显示状态效果图标（2秒）
		if (StatusEffect)
		{
			StatusEffect->ApplyEffect(
				UHealingIndicatorEffect::StaticClass(),
				Owner,
				2.0f  // 显示2秒
			);
		}
		break;

	case EItemType::StaminaPotion:
		// 先应用体力恢复效果
		if (UStaminaComponent* Stamina = Owner->FindComponentByClass<UStaminaComponent>())
		{
			Stamina->RestoreStamina(Item.EffectValue);
		}
		// 再显示状态效果图标（2秒）
		if (StatusEffect)
		{
			StatusEffect->ApplyEffect(
				UStaminaIndicatorEffect::StaticClass(),
				Owner,
				2.0f  // 显示2秒
			);
		}
		break;

	case EItemType::AttackBuff:
		if (StatusEffect)
		{
			// 通过状态效果系统应用攻击Buff
			StatusEffect->ApplyEffect(
				UAttackBuffEffect::StaticClass(),
				Owner,
				Item.EffectDuration
			);
		}
		break;

	case EItemType::DefenseBuff:
		if (StatusEffect)
		{
			// 通过状态效果系统应用防御Buff
			StatusEffect->ApplyEffect(
				UDefenseBuffEffect::StaticClass(),
				Owner,
				Item.EffectDuration
			);
		}
		break;

	default:
		break;
	}
}

bool UInventoryComponent::AddItemByType(EItemType Type, int32 Amount)
{
	// 查找对应类型的槽位
	for (int32 i = 0; i < ItemSlots.Num(); i++)
	{
		if (ItemSlots[i].ItemType == Type)
		{
			// 计算可添加的数量
			int32 SpaceLeft = ItemSlots[i].MaxCount - ItemSlots[i].CurrentCount;
			if (SpaceLeft <= 0)
			{
				return false;  // 已满
			}

			int32 ActualAdd = FMath::Min(Amount, SpaceLeft);
			ItemSlots[i].CurrentCount += ActualAdd;

			// 广播数量变化
			OnItemCountChanged.Broadcast(i, ItemSlots[i].CurrentCount);
			return true;
		}
	}
	return false;  // 未找到对应槽位
}
