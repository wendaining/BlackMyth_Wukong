// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SaveMenuWidget.generated.h"

/**
 * 存档界面 Widget。
 * 负责将当前游戏状态写入指定存档槽。
 */
UCLASS()
class BLACKMYTH_API USaveMenuWidget : public UUserWidget {
    GENERATED_BODY()

public:
    /** 点击存档槽（1~3）。 */
    UFUNCTION(BlueprintCallable, Category = "Save")
    void OnSaveSlotClicked(int32 SlotIndex);
};

