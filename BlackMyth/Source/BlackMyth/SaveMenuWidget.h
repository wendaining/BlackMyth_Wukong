// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "SaveMenuWidget.generated.h"

/**
 * 存档界面 Widget。
 * 负责将当前游戏状态写入指定存档槽。
 */
UCLASS()
class BLACKMYTH_API USaveMenuWidget : public UUserWidget {
    GENERATED_BODY()

protected:
    // 当 Widget 被创建并加入界面时调用
    virtual void NativeConstruct() override;

    void UpdateSlotInfo(int32 SlotIndex, UTextBlock* Text);

public:
    // 绑定 WBP 中的输入框
    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* SaveNameTextBox;

    /** 点击存档槽（1~3）。 */
    UFUNCTION(BlueprintCallable, Category = "Save")
    void OnSaveSlotClicked(int32 SlotIndex);

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SaveSlot1Text;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SaveSlot2Text;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SaveSlot3Text;
};

