// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "EnemySpawner.h"
#include "SaveMenuWidget.generated.h"

/**
 * 存档界面 Widget。
 * 负责将当前游戏状态写入指定存档槽。
 */
UCLASS()
class BLACKMYTH_API USaveMenuWidget : public UUserWidget {
	GENERATED_BODY()

protected:
	/** Widget 构建完成后调用，用于刷新存档信息。 */
	virtual void NativeConstruct() override;

	/** 更新指定存档槽的显示文本。 */
	void UpdateSlotInfo(int32 SlotIndex, UTextBlock* Text);

public:
	/** 存档名称输入框。 */
	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* SaveNameTextBox;

	/** 点击存档槽（1~3）。 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void OnSaveSlotClicked(int32 SlotIndex);

	/** 存档槽 1 显示文本。 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SaveSlot1Text;

	/** 存档槽 2 显示文本。 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SaveSlot2Text;

	/** 存档槽 3 显示文本。 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SaveSlot3Text;
};