// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TempleMenuWidget.generated.h"

/**
 * 土地庙主菜单控件
 * 提供传送、交易和退出等功能选项
 * 作为土地庙交互的主入口界面
 */
UCLASS()
class BLACKMYTH_API UTempleMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     * 传送按钮点击回调
     * 打开传送菜单，显示所有可用的土地庙传送点
     */
    UFUNCTION(BlueprintCallable)
    void OnTeleportClicked();

    /**
     * 交易按钮点击回调
     * 打开交易菜单，允许玩家进行物品交易
     */
    UFUNCTION(BlueprintCallable)
    void OnTradeClicked();

    /**
     * 退出按钮点击回调
     * 关闭土地庙菜单，恢复游戏运行状态
     */
    UFUNCTION(BlueprintCallable)
    void OnQuitClicked();
};