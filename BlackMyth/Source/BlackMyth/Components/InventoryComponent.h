// 背包组件 - 管理玩家的物品背包

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Items/ItemTypes.h"
#include "InventoryComponent.generated.h"

// 委托声明
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemCountChanged, int32, SlotIndex, int32, NewCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemUsed, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectedSlotChanged, int32, NewSlotIndex);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLACKMYTH_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// === 物品槽位 ===

	/** 物品槽位数组（默认4个：血药、体力药、攻击Buff、防御Buff） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FItemSlot> ItemSlots;

	/** 当前选中的槽位索引 */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 SelectedSlotIndex = 0;

	// === 核心接口 ===

	/** 使用指定槽位的物品（返回是否成功） */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseItem(int32 SlotIndex);

	/** 使用当前选中的物品 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseSelectedItem();

	/** 切换选中的槽位 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SelectSlot(int32 SlotIndex);

	/** 选择下一个槽位 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SelectNextSlot();

	/** 选择上一个槽位 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SelectPreviousSlot();

	/** 补充所有物品（土地庙休息时调用） */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefillAllItems();

	// === 查询接口 ===

	/** 获取指定类型物品的数量 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCount(EItemType Type) const;

	/** 获取指定槽位的物品 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	const FItemSlot& GetItemSlot(int32 SlotIndex) const;

	/** 获取槽位数量 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetSlotCount() const { return ItemSlots.Num(); }

	// === 委托 ===

	/** 物品数量变化时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemCountChanged OnItemCountChanged;

	/** 物品使用时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnItemUsed OnItemUsed;

	/** 选中槽位变化时触发 */
	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnSelectedSlotChanged OnSelectedSlotChanged;

protected:
	virtual void BeginPlay() override;

private:
	/** 执行物品效果 */
	void ApplyItemEffect(const FItemSlot& Item);

	/** 空槽位（用于返回无效索引时） */
	static FItemSlot EmptySlot;
};
