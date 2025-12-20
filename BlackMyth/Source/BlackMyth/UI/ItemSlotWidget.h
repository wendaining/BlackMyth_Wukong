// 物品槽位 Widget - 显示单个物品图标、数量和选中状态

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../Items/ItemTypes.h"
#include "ItemSlotWidget.generated.h"

class UImage;
class UTextBlock;
class UBorder;
class UWidgetAnimation;

/**
 * 单个物品槽位 Widget
 * 显示物品图标、数量和选中状态
 */
UCLASS()
class BLACKMYTH_API UItemSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========== 公共接口 ==========

	/** 设置物品数据（一次性更新所有显示） */
	UFUNCTION(BlueprintCallable, Category = "ItemSlot")
	void SetItemData(const FItemSlot& ItemData);

	/** 设置物品图标 */
	UFUNCTION(BlueprintCallable, Category = "ItemSlot")
	void SetItemIcon(UTexture2D* IconTexture);

	/** 设置物品数量显示 (格式: "当前/最大") */
	UFUNCTION(BlueprintCallable, Category = "ItemSlot")
	void SetItemCount(int32 CurrentCount, int32 MaxCount);

	/** 设置物品名称 */
	UFUNCTION(BlueprintCallable, Category = "ItemSlot")
	void SetItemName(const FText& Name);

	/** 设置选中状态（控制高亮边框显示） */
	UFUNCTION(BlueprintCallable, Category = "ItemSlot")
	void SetSelected(bool bSelected);

	/** 播放使用动画（物品使用时的闪烁效果） */
	UFUNCTION(BlueprintCallable, Category = "ItemSlot")
	void PlayUseAnimation();

	/** 获取是否被选中 */
	UFUNCTION(BlueprintPure, Category = "ItemSlot")
	bool IsSelected() const { return bIsSelected; }

protected:
	virtual void NativeConstruct() override;

	// ========== UI 控件绑定 ==========
	// 蓝图中的 Widget 组件需要与这里的变量名匹配才能自动绑定

	/** 物品图标 (在蓝图 Designer 中命名为 "ItemIcon") */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* ItemIcon;

	/** 物品数量文本 (在蓝图 Designer 中命名为 "ItemCount") */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ItemCount;

	/** 物品名称文本 (在蓝图 Designer 中命名为 "ItemName") */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* ItemName;

	/** 选中高亮边框 (在蓝图 Designer 中命名为 "SelectionBorder") */
	UPROPERTY(meta = (BindWidgetOptional))
	UBorder* SelectionBorder;

	/** 背景图片 (在蓝图 Designer 中命名为 "Background") */
	UPROPERTY(meta = (BindWidgetOptional))
	UImage* Background;

	/** 使用动画（在蓝图中创建名为 "UseAnimation" 的动画） */
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	UWidgetAnimation* UseAnimation;

	// ========== 样式配置 ==========

	/** 选中状态边框颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor SelectedBorderColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f);  // 金色

	/** 未选中状态边框颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor NormalBorderColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.5f);  // 半透明灰

	/** 物品耗尽时的图标颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor EmptyItemColor = FLinearColor(0.4f, 0.4f, 0.4f, 0.6f);  // 暗灰

	/** 物品可用时的图标颜色 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FLinearColor AvailableItemColor = FLinearColor::White;

private:
	/** 当前是否被选中 */
	bool bIsSelected = false;

	/** 更新视觉效果 */
	void UpdateVisuals();
};
