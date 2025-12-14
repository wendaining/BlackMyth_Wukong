// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadMenuWidget.generated.h"

/**
 * 读档界面 Widget。
 * 负责从指定存档槽恢复游戏状态。
 */
UCLASS()
class BLACKMYTH_API ULoadMenuWidget : public UUserWidget {
    GENERATED_BODY()

public:
    /** 点击读档槽（1~3）。 */
    UFUNCTION(BlueprintCallable, Category = "Load")
    void OnLoadSlotClicked(int32 SlotIndex);
};

