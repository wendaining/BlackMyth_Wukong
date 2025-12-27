// 背包栏 Widget - 管理多个物品槽位的容器

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryBarWidget.generated.h"

class UItemSlotWidget;
class UInventoryComponent;
class AWukongCharacter;

/**
 * 背包栏 Widget - 管理多个物品槽位的容器
 */
UCLASS()
class BLACKMYTH_API UInventoryBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 手动刷新所有槽位显示（一般不需要调用，自动更新） */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshAllSlots();

	/** 获取指定索引的槽位 Widget */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UItemSlotWidget* GetItemSlot(int32 SlotIndex) const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ========== UI 控件绑定 ==========
	// 在蓝图 Designer 中，将 ItemSlot 子组件命名为对应名称即可自动绑定

	/** 物品槽位 0 (血药) */
	UPROPERTY(meta = (BindWidgetOptional))
	UItemSlotWidget* ItemSlot_0;

	/** 物品槽位 1 (体力药) */
	UPROPERTY(meta = (BindWidgetOptional))
	UItemSlotWidget* ItemSlot_1;

	/** 物品槽位 2 (攻击Buff - 怒火丹) */
	UPROPERTY(meta = (BindWidgetOptional))
	UItemSlotWidget* ItemSlot_2;

	/** 物品槽位 3 (防御Buff - 金刚丹) */
	UPROPERTY(meta = (BindWidgetOptional))
	UItemSlotWidget* ItemSlot_3;

	// ========== 配置属性 ==========

	/** 是否播放使用动画 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	bool bPlayUseAnimation = true;

	/** 使用物品时播放的音效 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Audio")
	USoundBase* UseItemSound;

	/** 使用音效音量 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Audio")
	float UseItemSoundVolume = 0.5f;

private:
	// ========== 引用缓存 ==========

	/** 玩家角色引用 */
	UPROPERTY()
	AWukongCharacter* PlayerCharacter;

	/** 背包组件引用 */
	UPROPERTY()
	UInventoryComponent* InventoryComp;

	/** 槽位数组（方便遍历） */
	TArray<UItemSlotWidget*> ItemSlots;

	// ========== 初始化函数 ==========

	/** 初始化所有槽位（获取引用、更新显示、绑定委托） */
	void InitializeItemSlots();

	/** 更新单个槽位显示 */
	void UpdateItemSlotDisplay(int32 SlotIndex);

	// ========== 委托处理函数 ==========

	/** 物品数量变化时的回调 */
	UFUNCTION()
	void HandleItemCountChanged(int32 SlotIndex, int32 NewCount);

	/** 物品使用时的回调 */
	UFUNCTION()
	void HandleItemUsed(int32 SlotIndex);

	/** 选中槽位变化时的回调 */
	UFUNCTION()
	void HandleSelectedSlotChanged(int32 NewSlotIndex);
};
