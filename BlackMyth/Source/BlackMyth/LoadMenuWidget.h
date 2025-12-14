// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "LoadMenuWidget.generated.h"

/**
 * 读档界面 Widget。
 * 显示已有存档信息，并从指定槽位恢复游戏状态。
 */
UCLASS()
class BLACKMYTH_API ULoadMenuWidget : public UUserWidget {
    GENERATED_BODY()

protected:
    /** Widget 构建完成后调用，用于刷新存档信息。 */
    virtual void NativeConstruct() override;

    /** 更新指定存档槽的显示文本。 */
    void UpdateSlotInfo(int32 SlotIndex, UTextBlock* Text);

public:
    /** 存档槽 1 显示文本 */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* LoadSlot1Text;

    /** 存档槽 2 显示文本 */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* LoadSlot2Text;

    /** 存档槽 3 显示文本 */
    UPROPERTY(meta = (BindWidget))
    UTextBlock* LoadSlot3Text;

    /** 点击读档槽（1~3）。 */
    UFUNCTION(BlueprintCallable, Category = "Load")
    void OnLoadSlotClicked(int32 SlotIndex);
};
