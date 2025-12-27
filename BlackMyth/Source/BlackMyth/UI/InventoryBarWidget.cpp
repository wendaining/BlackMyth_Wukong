// 背包栏 Widget 实现

#include "InventoryBarWidget.h"
#include "ItemSlotWidget.h"
#include "../Components/InventoryComponent.h"
#include "../WukongCharacter.h"
#include "Kismet/GameplayStatics.h"

void UInventoryBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 构建槽位数组
	ItemSlots.Empty();
	ItemSlots.Add(ItemSlot_0);
	ItemSlots.Add(ItemSlot_1);
	ItemSlots.Add(ItemSlot_2);
	ItemSlots.Add(ItemSlot_3);

	// 初始化所有槽位
	InitializeItemSlots();
}

void UInventoryBarWidget::NativeDestruct()
{
	// 解绑委托（防止悬空指针）
	if (InventoryComp)
	{
		InventoryComp->OnItemCountChanged.RemoveDynamic(this, &UInventoryBarWidget::HandleItemCountChanged);
		InventoryComp->OnItemUsed.RemoveDynamic(this, &UInventoryBarWidget::HandleItemUsed);
		InventoryComp->OnSelectedSlotChanged.RemoveDynamic(this, &UInventoryBarWidget::HandleSelectedSlotChanged);
	}

	Super::NativeDestruct();
}

void UInventoryBarWidget::InitializeItemSlots()
{
	// 1. 获取玩家角色
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryBarWidget: Cannot get owning player controller"));
		return;
	}

	PlayerCharacter = Cast<AWukongCharacter>(PC->GetPawn());
	if (!PlayerCharacter)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryBarWidget: Cannot cast pawn to WukongCharacter"));
		return;
	}

	// 2. 获取背包组件
	InventoryComp = PlayerCharacter->GetInventoryComponent();
	if (!InventoryComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryBarWidget: WukongCharacter has no InventoryComponent"));
		return;
	}

	// 3. 初始化所有槽位显示
	for (int32 i = 0; i < ItemSlots.Num(); i++)
	{
		UpdateItemSlotDisplay(i);
	}

	// 4. 绑定委托事件
	InventoryComp->OnItemCountChanged.AddDynamic(this, &UInventoryBarWidget::HandleItemCountChanged);
	InventoryComp->OnItemUsed.AddDynamic(this, &UInventoryBarWidget::HandleItemUsed);
	InventoryComp->OnSelectedSlotChanged.AddDynamic(this, &UInventoryBarWidget::HandleSelectedSlotChanged);

	// 5. 设置初始选中状态
	HandleSelectedSlotChanged(InventoryComp->SelectedSlotIndex);

	UE_LOG(LogTemp, Log, TEXT("InventoryBarWidget: Initialized successfully with %d slots"), ItemSlots.Num());
}

void UInventoryBarWidget::UpdateItemSlotDisplay(int32 SlotIndex)
{
	// 检查索引有效性
	if (SlotIndex < 0 || SlotIndex >= ItemSlots.Num())
	{
		return;
	}

	UItemSlotWidget* SlotWidget = ItemSlots[SlotIndex];
	if (!SlotWidget)
	{
		return;
	}

	if (!InventoryComp)
	{
		return;
	}

	// 从 InventoryComponent 获取槽位数据
	const FItemSlot& ItemData = InventoryComp->GetItemSlot(SlotIndex);

	// 更新槽位显示
	SlotWidget->SetItemData(ItemData);
}

void UInventoryBarWidget::RefreshAllSlots()
{
	for (int32 i = 0; i < ItemSlots.Num(); i++)
	{
		UpdateItemSlotDisplay(i);
	}
}

UItemSlotWidget* UInventoryBarWidget::GetItemSlot(int32 SlotIndex) const
{
	if (SlotIndex >= 0 && SlotIndex < ItemSlots.Num())
	{
		return ItemSlots[SlotIndex];
	}
	return nullptr;
}

// ========== 委托处理函数 ==========

void UInventoryBarWidget::HandleItemCountChanged(int32 SlotIndex, int32 NewCount)
{
	// 更新对应槽位的显示
	UpdateItemSlotDisplay(SlotIndex);
}

void UInventoryBarWidget::HandleItemUsed(int32 SlotIndex)
{
	// 检查索引有效性
	if (SlotIndex < 0 || SlotIndex >= ItemSlots.Num())
	{
		return;
	}

	UItemSlotWidget* SlotWidget = ItemSlots[SlotIndex];
	if (!SlotWidget)
	{
		return;
	}

	// 播放使用动画
	if (bPlayUseAnimation)
	{
		SlotWidget->PlayUseAnimation();
	}

	// 播放使用音效
	if (UseItemSound)
	{
		UGameplayStatics::PlaySound2D(this, UseItemSound, UseItemSoundVolume);
	}
}

void UInventoryBarWidget::HandleSelectedSlotChanged(int32 NewSlotIndex)
{
	// 清除所有槽位的选中状态
	for (int32 i = 0; i < ItemSlots.Num(); i++)
	{
		UItemSlotWidget* SlotWidget = ItemSlots[i];
		if (SlotWidget)
		{
			SlotWidget->SetSelected(i == NewSlotIndex);
		}
	}
}
